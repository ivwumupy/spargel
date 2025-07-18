#include <spargel/config.h>
#include <spargel/render/ui_renderer.h>

namespace spargel::render {

#if SPARGEL_IS_MACOS
    base::UniquePtr<UIRenderer> makeMetalUIRenderer(gpu::GPUContext* context,
                                                    resource::ResourceManager* resource_manager);
#endif

    base::UniquePtr<UIRenderer> makeUIRenderer(gpu::GPUContext* context,
                                               resource::ResourceManager* resource_manager) {
#if SPARGEL_IS_MACOS
        return makeMetalUIRenderer(context, resource_manager);
#endif
        return nullptr;
    }
}  // namespace spargel::render
