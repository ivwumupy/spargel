#pragma once

#include <spargel/base/hash_map.h>
#include <spargel/gpu/metal_context.h>
#include <spargel/render/ui_renderer.h>
#include <spargel/text/font.h>

//
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
#include <dispatch/dispatch.h>

namespace spargel::render {
    class UIRendererMetal final : public UIRenderer {
    public:
        UIRendererMetal(gpu::GPUContext* context, resource::ResourceManager* resource_manager,
                        text::TextShaper* text_shaper);
        ~UIRendererMetal();

        void setLayer(CAMetalLayer* layer) {
            layer_ = layer;
            layer_.device = device_;
            layer_.pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
        }

        void render(UIScene const& scene) override;
        void uploadBitmap(TextureHandle handle, text::Bitmap const& bitmap) override;

        gpu::MetalContext* metal_context() { return static_cast<gpu::MetalContext*>(context()); }

    private:
        // TODO: We should not purge the buffers by delta-time.
        struct BufferItem {
            bool olderThan(base::Optional<BufferItem> const& other) const;

            id<MTLBuffer> object = nullptr;
            // Timestamp of the last use, in nanoseconds.
            u64 last_used = 0;
            bool in_use = false;
        };
        struct BufferPool {
            BufferPool(gpu::MetalContext* ctx) : context{ctx} {}

            id<MTLBuffer> request(usize length);
            void purge(u64 now);
            void putBack(id<MTLBuffer> buffer);

            gpu::MetalContext* context;
            // Free buffers.
            base::Vector<BufferItem> buffers;
            // Timestamp of the last purge, in nanoseconds.
            u64 last_purged = 0;
        };
        struct GrowingBuffer {
            void request(usize length);

            gpu::MetalContext* context = nullptr;
            id<MTLBuffer> object = nullptr;
        };

        void initPipeline();
        void initTexture();

        id<MTLDevice> device_;
        id<MTLCommandQueue> queue_;
        CAMetalLayer* layer_;
        resource::ResourceManager* resource_manager_;

        id<MTLLibrary> library_;
        id<MTLFunction> sdf_vert_;
        id<MTLFunction> sdf_frag_;
        id<MTLRenderPipelineState> sdf_pipeline_;

        MTLRenderPassDescriptor* pass_desc_;

        BufferPool buffer_pool_;
        GrowingBuffer scene_commands_buffer_;
        GrowingBuffer scene_data_buffer_;

        id<MTLTexture> texture_;

        // base::HashMap<>;
    };
    base::UniquePtr<UIRenderer> makeMetalUIRenderer(gpu::GPUContext* context,
                                                    resource::ResourceManager* resource_manager,
                                                    text::TextShaper* shaper);
}  // namespace spargel::render
