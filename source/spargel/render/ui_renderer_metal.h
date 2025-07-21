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
        static constexpr bool use_compute = true;

        UIRendererMetal(gpu::GPUContext* context, text::TextShaper* text_shaper);
        ~UIRendererMetal();

        void setLayer(CAMetalLayer* layer) {
            layer_ = layer;
            layer_.device = device_;
            layer_.pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
            if (use_compute) {
                layer_.framebufferOnly = false;
            }
        }

        void render(UIScene const& scene) override;
        void uploadBitmap(TextureHandle handle, text::Bitmap const& bitmap) override;

        gpu::MetalContext* metal_context() { return static_cast<gpu::MetalContext*>(context()); }

        //
        id<MTLCommandBuffer> renderToTexture(UIScene const& scene, id<MTLTexture> texture);
        id<MTLCommandBuffer> renderToTextureRender(UIScene const& scene, id<MTLTexture> texture);
        id<MTLCommandBuffer> renderToTextureCompute(UIScene const& scene, id<MTLTexture> texture);
        id<MTLCommandBuffer> renderToTextureComputeV2(UIScene const& scene, id<MTLTexture> texture);

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

        id<MTLLibrary> library_;
        id<MTLFunction> sdf_vert_;
        id<MTLFunction> sdf_frag_;
        id<MTLFunction> sdf_comp_;
        id<MTLFunction> sdf_comp_v2_;
        id<MTLFunction> sdf_binning_;
        id<MTLRenderPipelineState> sdf_pipeline_;
        id<MTLComputePipelineState> sdf_comp_pipeline_;
        id<MTLComputePipelineState> sdf_comp_v2_pipeline_;
        id<MTLComputePipelineState> sdf_binning_pipeline_;

        MTLRenderPassDescriptor* pass_desc_;

        BufferPool buffer_pool_;
        GrowingBuffer scene_commands_buffer_;
        GrowingBuffer scene_data_buffer_;

        id<MTLTexture> texture_;

        GrowingBuffer bin_slots_buffer_;
        GrowingBuffer bin_alloc_buffer_;
    };
    base::UniquePtr<UIRenderer> makeMetalUIRenderer(gpu::GPUContext* context,
                                                    text::TextShaper* shaper);
}  // namespace spargel::render
