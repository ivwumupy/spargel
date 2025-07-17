#pragma once

#include <spargel/base/unique_ptr.h>

namespace spargel::gpu {
    enum class GPUBackend {
        directx,
        metal,
        opengl,
        vulkan,
    };
    class GPUContext {
    public:
        virtual ~GPUContext() = default;
    };
    base::UniquePtr<GPUContext> makeContext(GPUBackend backend);
}  // namespace spargel::gpu
