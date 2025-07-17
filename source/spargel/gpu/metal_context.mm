#include <spargel/gpu/metal_context.h>

namespace spargel::gpu {
    base::UniquePtr<GPUContext> makeMetalContext() {
        return base::makeUnique<MetalContext>();
    }
    void MetalContext::init() {
        device_ = MTLCreateSystemDefaultDevice();
    }
    void MetalContext::cleanup() {
        [device_ release];
    }
}  // namespace spargel::gpu
