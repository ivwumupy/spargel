#include <spargel/config.h>
#include <spargel/gpu/gpu3.h>

#if SPARGEL_GPU_ENABLE_METAL
namespace spargel::gpu {
    base::unique_ptr<Device> make_device_metal();
}
#endif

namespace spargel::gpu {

    base::unique_ptr<Device> makeDevice(DeviceKind kind) {
        switch (kind) {
#if SPARGEL_GPU_ENABLE_METAL
        case DeviceKind::metal:
            return make_device_metal();
#endif
        default:
            return nullptr;
        }
    }

}  // namespace spargel::gpu
