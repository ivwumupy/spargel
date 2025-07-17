#include <spargel/render/ui_renderer_metal.h>

namespace spargel::render {
    base::UniquePtr<UIRenderer> makeMetalUIRenderer(gpu::GPUContext* context) {
        return base::makeUnique<UIRendererMetal>(context);
    }
}  // namespace spargel::render
