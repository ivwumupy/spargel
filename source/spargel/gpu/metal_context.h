#pragma once

#include <spargel/gpu/gpu_context.h>

//
#import <Metal/Metal.h>

namespace spargel::gpu {
    class MetalContext final : public GPUContext {
    public:
        MetalContext() { init(); }
        ~MetalContext() { cleanup(); }

        id<MTLDevice> device() { return device_; }
        id<MTLCommandQueue> queue() { return queue_; }

    private:
        void init();
        void cleanup();

        id<MTLDevice> device_;
        id<MTLCommandQueue> queue_;
    };
    base::UniquePtr<GPUContext> makeMetalContext();
}  // namespace spargel::gpu
