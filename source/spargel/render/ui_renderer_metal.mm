#include <spargel/base/check.h>
#include <spargel/base/checked_convert.h>
#include <spargel/base/types.h>
#include <spargel/render/ui_renderer_metal.h>
#include <spargel/render/ui_scene.h>

//
#include <stdio.h>

namespace spargel::render {
    base::UniquePtr<UIRenderer> makeMetalUIRenderer(gpu::GPUContext* context, resource::ResourceManager* resource_manager) {
        return base::makeUnique<UIRendererMetal>(context, resource_manager);
    }
    void UIRendererMetal::initPipeline() {
        NSError* error = nullptr;

        // Load shaders.
        auto shaders_resource =
            resource_manager_->open(
                resource::ResourceId("source/spargel/render/ui_shaders.metallib"));
        spargel_check(shaders_resource.hasValue());
        auto shaders_data = shaders_resource.value()->getSpan();
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
        auto ppl_desc =
            [[MTLRenderPipelineDescriptor alloc] init];
        ppl_desc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
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
    void UIRendererMetal::render(UIScene const& scene) {
        spargel_check(layer_);
        auto command_buffer = [queue_ commandBuffer];
        auto drawable = [layer_ nextDrawable];
        pass_desc_.colorAttachments[0].texture = drawable.texture;
        auto encoder =
            [command_buffer renderCommandEncoderWithDescriptor:pass_desc_];

        struct {
            u32 cmd_count;
        } uniform_data = {base::checkedConvert<u32>(scene.commands().count())};

        [encoder setRenderPipelineState:sdf_pipeline_];
        [encoder setFragmentBytes:&uniform_data
                           length:sizeof(uniform_data)
                          atIndex:0];
        auto commands_bytes = scene.commands().asBytes();
        auto data_bytes = scene.data().asBytes();
        [encoder setFragmentBytes:commands_bytes.data() length:commands_bytes.count() atIndex:1];
        [encoder setFragmentBytes:data_bytes.data() length:data_bytes.count() atIndex:2];

        // we use one triangle
        [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];

        [encoder endEncoding];
        [command_buffer presentDrawable:drawable];
        [command_buffer commit];
    }
    UIRendererMetal::~UIRendererMetal() {
        [pass_desc_ release];
        [sdf_pipeline_ release];
        [sdf_frag_ release];
        [sdf_vert_ release];
        [library_ release];
    }
}  // namespace spargel::render
