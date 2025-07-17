#include <spargel/gpu/metal_context.h>

namespace spargel::gpu {
    base::UniquePtr<GPUContext> makeMetalContext() {
        return base::makeUnique<MetalContext>();
    }
    void MetalContext::init() {
        device_ = MTLCreateSystemDefaultDevice();
        queue_ = [device_ newCommandQueue];
    }
    void MetalContext::cleanup() {
        [queue_ release];
        [device_ release];
    }
}  // namespace spargel::gpu
