#pragma once

#include "spargel/gpu/gpu2.h"
#include "spargel/base/unique_ptr.h"

#import <Metal/Metal.h>

namespace spargel::gpu2 {
    
    class MetalDevice;
    class MetalCommandQueue;
    
    class MetalDevice final : public Device {
    public:

    private:
        id<MTLDevice> device_;
    };

    base::UniquePtr<Device> createMetalDevice();

}
