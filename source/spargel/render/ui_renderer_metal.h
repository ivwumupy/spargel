#pragma once

#include <spargel/gpu/metal_context.h>
#include <spargel/render/ui_renderer.h>

//
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

namespace spargel::render {
    class UIRendererMetal final : public UIRenderer {
    public:
        explicit UIRendererMetal(gpu::GPUContext* context)
            : UIRenderer{context},
              device_{metal_context()->device()},
              queue_{metal_context()->queue()} {}

        void setLayer(CAMetalLayer* layer) {
            layer_ = layer;
            layer_.device = device_;
        }

        void render() override;

        gpu::MetalContext* metal_context() { return static_cast<gpu::MetalContext*>(context()); }

    private:
        id<MTLDevice> device_;
        id<MTLCommandQueue> queue_;
        CAMetalLayer* layer_;
    };
    base::UniquePtr<UIRenderer> makeMetalUIRenderer(gpu::GPUContext* context);
}  // namespace spargel::render
