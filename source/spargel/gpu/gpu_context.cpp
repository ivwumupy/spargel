#include <spargel/config.h>
#include <spargel/gpu/gpu_context.h>

namespace spargel::gpu {

#if SPARGEL_IS_MACOS
    base::UniquePtr<GPUContext> makeMetalContext();
#endif

    base::UniquePtr<GPUContext> makeContext(GPUBackend backend) {
        switch (backend) {
#if SPARGEL_IS_MACOS
        case GPUBackend::metal:
            return makeMetalContext();
#endif
        default:
            spargel_panic("backend not supported");
        }
    }
}  // namespace spargel::gpu
