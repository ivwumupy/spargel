#include <spargel/base/check.h>
#include <spargel/base/checked_convert.h>
#include <spargel/base/types.h>
#include <spargel/render/ui_renderer_metal.h>
#include <spargel/render/ui_scene.h>

#include "spargel/render/metal_shaders.h"

//
#include <time.h>

namespace spargel::render {
    base::UniquePtr<UIRenderer> makeMetalUIRenderer(gpu::GPUContext* context,
                                                    resource::ResourceManager* resource_manager,
                                                    text::TextShaper* shaper) {
        return base::makeUnique<UIRendererMetal>(context, resource_manager, shaper);
    }

    UIRendererMetal::UIRendererMetal(gpu::GPUContext* context,
                                     resource::ResourceManager* resource_manager,
                                     text::TextShaper* text_shaper)
        : UIRenderer{context, text_shaper},
          device_{metal_context()->device()},
          queue_{metal_context()->queue()},
          resource_manager_{resource_manager},
          buffer_pool_{metal_context()} {
        initPipeline();
        initTexture();
        scene_commands_buffer_.context = metal_context();
        scene_data_buffer_.context = metal_context();
    }
    void UIRendererMetal::initPipeline() {
        NSError* error = nullptr;

        // Load shaders.
        // auto shaders_resource = resource_manager_->open(
        //     resource::ResourceId("source/spargel/render/ui_shaders.metallib"));
        // spargel_check(shaders_resource.hasValue());
        // auto shaders_data = shaders_resource.value()->getSpan();
        auto shaders_data = METAL_SHADERS.asSpan();
        auto shaders_dispatch_data =
            dispatch_data_create(shaders_data.data(), shaders_data.count(),
                                 dispatch_get_main_queue(), DISPATCH_DATA_DESTRUCTOR_DEFAULT);

        // Create shader library.
        library_ = [device_ newLibraryWithData:shaders_dispatch_data error:&error];
        spargel_check(!error);
        dispatch_release(shaders_dispatch_data);

        // Create functions.
        sdf_vert_ = [library_ newFunctionWithName:@"sdf_vert"];
        sdf_frag_ = [library_ newFunctionWithName:@"sdf_frag"];
        spargel_check(sdf_vert_);
        spargel_check(sdf_frag_);

        // Create pipeline.
        auto ppl_desc = [[MTLRenderPipelineDescriptor alloc] init];
        ppl_desc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
        ppl_desc.colorAttachments[0].blendingEnabled = true;
        ppl_desc.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
        ppl_desc.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
        ppl_desc.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
        ppl_desc.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
        ppl_desc.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
        ppl_desc.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOne;

        ppl_desc.vertexFunction = sdf_vert_;
        ppl_desc.fragmentFunction = sdf_frag_;
        sdf_pipeline_ = [device_ newRenderPipelineStateWithDescriptor:ppl_desc error:&error];
        spargel_check(!error);
        [ppl_desc release];

        // Prepare render pass descriptor.
        pass_desc_ = [[MTLRenderPassDescriptor alloc] init];
        pass_desc_.colorAttachments[0].loadAction = MTLLoadActionClear;
        pass_desc_.colorAttachments[0].storeAction = MTLStoreActionStore;
        pass_desc_.colorAttachments[0].clearColor = MTLClearColorMake(0, 1, 1, 1);
    }
    void UIRendererMetal::initTexture() {
        auto desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatA8Unorm
                                                                       width:ATLAS_SIZE
                                                                      height:ATLAS_SIZE
                                                                   mipmapped:false];
        desc.storageMode = MTLStorageModeShared;
        texture_ = [device_ newTextureWithDescriptor:desc];
    }
    void UIRendererMetal::render(UIScene const& scene) {
        spargel_check(layer_);
        auto command_buffer = [queue_ commandBuffer];

        auto commands_bytes = scene.commands().asBytes();
        auto data_bytes = scene.data().asBytes();

        scene_commands_buffer_.request(commands_bytes.count());
        scene_data_buffer_.request(data_bytes.count());
        // auto commands_buffer = buffer_pool_.request(commands_bytes.count());
        // auto data_buffer = buffer_pool_.request(data_bytes.count());

        memcpy(scene_commands_buffer_.object.contents, commands_bytes.data(),
               commands_bytes.count());
        memcpy(scene_data_buffer_.object.contents, data_bytes.data(), data_bytes.count());

        auto drawable = [layer_ nextDrawable];
        pass_desc_.colorAttachments[0].texture = drawable.texture;
        auto encoder = [command_buffer renderCommandEncoderWithDescriptor:pass_desc_];

        struct {
            u32 cmd_count;
        } uniform_data = {base::checkedConvert<u32>(scene.commands().count())};

        [encoder setRenderPipelineState:sdf_pipeline_];
        [encoder setFragmentBytes:&uniform_data length:sizeof(uniform_data) atIndex:0];

        id<MTLBuffer> buffers[] = {scene_commands_buffer_.object, scene_data_buffer_.object};
        usize offsets[] = {0, 0};
        [encoder setFragmentBuffers:buffers offsets:offsets withRange:NSMakeRange(1, 2)];
        [encoder setFragmentTexture:texture_ atIndex:0];

        // we use one triangle
        [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];

        [encoder endEncoding];
        [command_buffer presentDrawable:drawable];
        [command_buffer commit];

        // TODO: Rewrite using completionHandler.
        // It seems that we need to add a mutex in BufferPool.
        [command_buffer waitUntilCompleted];
        // buffer_pool_.putBack(commands_buffer);
        // buffer_pool_.putBack(data_buffer);
        // spargel_log_info("done");
    }
    UIRendererMetal::~UIRendererMetal() {
        [pass_desc_ release];
        [sdf_pipeline_ release];
        [sdf_frag_ release];
        [sdf_vert_ release];
        [library_ release];
    }
    namespace {
        // Get the current time in nanoseconds.
        //
        // NOTE: This is not portable.
        // Both `clock_gettime_nsec_np` and `CLOCK_UPTIME_RAW` are Darwin extensions.
        u64 getTimestamp() { return clock_gettime_nsec_np(CLOCK_UPTIME_RAW); }
    }  // namespace
    id<MTLBuffer> UIRendererMetal::BufferPool::request(usize length) {
        auto now = getTimestamp();
        if (now - last_purged > 1e9) {
            purge(now);
        }
        base::Optional<BufferItem> candidate = base::nullopt;
        for (auto const& item : buffers) {
            if (!item.in_use && item.object.length >= length && item.olderThan(candidate)) {
                candidate = item;
            }
        }
        if (candidate.hasValue()) {
            candidate.value().last_used = now;
            candidate.value().in_use = true;
            return candidate.value().object;
        }
        auto buffer = context->createBuffer(length);
        buffers.emplace(buffer, now, true);
        return buffer;
    }
    void UIRendererMetal::BufferPool::purge(u64 now) {
        buffers.eraseIfFastWithDeleter(
            [this](BufferItem const& item) { return !item.in_use && item.last_used > last_purged; },
            [this](BufferItem const& item) { context->destroyBuffer(item.object); });
        last_purged = now;
    }
    void UIRendererMetal::BufferPool::putBack(id<MTLBuffer> buffer) {
        for (auto& item : buffers) {
            if (item.object == buffer) {
                item.in_use = false;
                break;
            }
        }
    }
    bool UIRendererMetal::BufferItem::olderThan(base::Optional<BufferItem> const& other) const {
        if (!other.hasValue()) {
            return true;
        }
        return last_used < other.value().last_used;
    }
    void UIRendererMetal::GrowingBuffer::request(usize length) {
        if (length == 0) {
            return;
        }
        if (!object) {
            object = context->createBuffer(length);
            return;
        }
        if (length <= object.length) {
            return;
        }
        context->destroyBuffer(object);
        object = context->createBuffer(length);
    }
    void UIRendererMetal::uploadBitmap(TextureHandle handle, text::Bitmap const& bitmap) {
        [texture_ replaceRegion:MTLRegionMake2D(handle.x, handle.y, handle.width, handle.height)
                    mipmapLevel:0
                      withBytes:bitmap.data.data()
                    bytesPerRow:bitmap.width];
    }
}  // namespace spargel::render
