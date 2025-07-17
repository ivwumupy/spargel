#pragma once

#include <spargel/gpu/metal_context.h>
#include <spargel/render/ui_renderer.h>

namespace spargel::render {
    class UIRendererMetal final : public UIRenderer {
    public:
        explicit UIRendererMetal(gpu::GPUContext* context) : UIRenderer{context} {}

        gpu::MetalContext* metal_context() { return static_cast<gpu::MetalContext*>(context()); }
    };
    base::UniquePtr<UIRenderer> makeMetalUIRenderer(gpu::GPUContext* context);
}  // namespace spargel::render
