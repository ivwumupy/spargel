#include "spargel/gpu/gpu_context.h"

#include "spargel/base/panic.h"
#include "spargel/base/unique_ptr.h"
#include "spargel/config.h"

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

    GPUBackend inferBackend() {
#if SPARGEL_IS_MACOS
        return GPUBackend::metal;
#endif
        spargel_panic("cannot determine backend");
    }

}  // namespace spargel::gpu
