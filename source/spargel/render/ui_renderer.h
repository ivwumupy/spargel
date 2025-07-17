#pragma once

#include <spargel/gpu/gpu_context.h>

namespace spargel::render {
    class UIRenderer {
    public:
        explicit UIRenderer(gpu::GPUContext* context) : context_{context} {}
        virtual ~UIRenderer() = default;

        gpu::GPUContext* context() { return context_; }

        virtual void render() = 0;

    private:
        gpu::GPUContext* context_;
    };
    base::UniquePtr<UIRenderer> makeUIRenderer(gpu::GPUContext* context);
}  // namespace spargel::render
