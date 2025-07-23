#pragma once

#import <Metal/Metal.h>

#include "spargel/base/unique_ptr.h"
#include "spargel/gpu/gpu2.h"

namespace spargel::gpu2 {

    class MetalDevice;
    class MetalCommandQueue;

    class MetalDevice final : public Device {
    public:
    private:
        id<MTLDevice> device_;
    };

    base::UniquePtr<Device> createMetalDevice();

}  // namespace spargel::gpu2
