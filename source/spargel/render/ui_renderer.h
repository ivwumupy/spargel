#pragma once

#include <spargel/gpu/gpu_context.h>
#include <spargel/resource/resource.h>

namespace spargel::render {
    class UIScene;

    class UIRenderer {
    public:
        virtual ~UIRenderer() = default;

        gpu::GPUContext* context() { return context_; }

        virtual void render(UIScene const& scene) = 0;

    protected:
        explicit UIRenderer(gpu::GPUContext* context) : context_{context} {}

    private:
        gpu::GPUContext* context_;
    };
    base::UniquePtr<UIRenderer> makeUIRenderer(gpu::GPUContext* context, resource::ResourceManager* resource_manager);
}  // namespace spargel::render
