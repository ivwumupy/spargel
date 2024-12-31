#include <spargel/gpu/gpu.h>
#include <spargel/resource/directory.h>

//
#include <stdio.h>

using namespace spargel;

static constexpr float arr1[] = {1.0, 2.0, 3.0};
static constexpr float arr2[] = {4.0, 5.0, 6.0};

int main() {
    auto resource_manager = resource::make_relative_manager();
    // auto device = gpu::makeDevice(gpu::DeviceKind::metal);
    auto device = gpu::makeDevice(gpu::DeviceKind::vulkan);

    auto buf1 = device->createBuffer(gpu::BufferUsage::storage,
                                     base::make_span<u8>(sizeof(arr1), (u8*)&arr1));
    auto buf2 = device->createBuffer(gpu::BufferUsage::storage,
                                     base::make_span<u8>(sizeof(arr2), (u8*)&arr2));
    auto buf3 =
        device->createBuffer(gpu::BufferUsage::storage | gpu::BufferUsage::map_read, sizeof(arr1));

    auto blob = resource_manager->open(resource::resource_id("shader.metallib"));
    auto library = device->createShaderLibrary(blob->getSpan());
    auto pipeline = device->createComputePipeline(library, "add_arrays");
    auto max_size = pipeline->maxGroupSize();
    spargel_log_info("maxGroupSize = %u", max_size);

    auto queue = device->createCommandQueue();
    auto cmdbuf = queue->createCommandBuffer();
    auto encoder = cmdbuf->beginComputePass();
    encoder->setComputePipeline(pipeline);
    encoder->setBuffer(buf1, {.apple = {.buffer_index = 0}});
    encoder->setBuffer(buf2, {.apple = {.buffer_index = 1}});
    encoder->setBuffer(buf3, {.apple = {.buffer_index = 2}});
    encoder->dispatch({.x = 1, .y = 1, .z = 1}, {.x = max_size > 3 ? 3 : max_size, .y = 1, .z = 1});
    cmdbuf->endComputePass(encoder);
    cmdbuf->submit();
    cmdbuf->wait();
    float* result = (float*)buf3->mapAddr();
    for (int i = 0; i < 3; i++) {
        printf("%.1f ", result[i]);
    }
    putchar('\n');
    return 0;
}
