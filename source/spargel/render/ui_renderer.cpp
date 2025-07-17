#include <spargel/config.h>
#include <spargel/render/ui_renderer.h>

namespace spargel::render {

#if SPARGEL_IS_MACOS
    base::UniquePtr<UIRenderer> makeMetalContext(gpu::GPUContext* context);
#endif

    base::UniquePtr<UIRenderer> makeUIRenderer(gpu::GPUContext* context) {
#if SPARGEL_IS_MACOS
        return makeMetalContext(context);
#endif
        return nullptr;
    }
}  // namespace spargel::render
