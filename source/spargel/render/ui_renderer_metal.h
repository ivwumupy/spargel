#pragma once

#include <spargel/gpu/metal_context.h>
#include <spargel/render/ui_renderer.h>

//
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
#include <dispatch/dispatch.h>

namespace spargel::render {
    class UIRendererMetal final : public UIRenderer {
    public:
        UIRendererMetal(gpu::GPUContext* context, resource::ResourceManager* resource_manager)
            : UIRenderer{context},
              device_{metal_context()->device()},
              queue_{metal_context()->queue()},
              resource_manager_{resource_manager} {
            initPipeline();
        }
        ~UIRendererMetal();

        void setLayer(CAMetalLayer* layer) {
            layer_ = layer;
            layer_.device = device_;
            layer_.pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
        }

        void render(UIScene const& scene) override;

        gpu::MetalContext* metal_context() { return static_cast<gpu::MetalContext*>(context()); }

    private:
        void initPipeline();

        id<MTLDevice> device_;
        id<MTLCommandQueue> queue_;
        CAMetalLayer* layer_;
        resource::ResourceManager* resource_manager_;

        id<MTLLibrary> library_;
        id<MTLFunction> sdf_vert_;
        id<MTLFunction> sdf_frag_;
        id<MTLRenderPipelineState> sdf_pipeline_;

        MTLRenderPassDescriptor* pass_desc_;
    };
    base::UniquePtr<UIRenderer> makeMetalUIRenderer(gpu::GPUContext* context, resource::ResourceManager* resource_manager);
}  // namespace spargel::render
