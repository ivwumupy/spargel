#include <spargel/base/command_line.h>
#include <spargel/gpu/gpu.h>
#include <spargel/resource/directory.h>

//
#include <stdio.h>

using namespace spargel;

static constexpr u32 count = 3;
static constexpr float arr1[] = {1.0, 2.0, 3.0};
static constexpr float arr2[] = {4.0, 5.0, 6.0};

int main(int argc, char* argv[]) {
    base::CommandLine cmdline(argc, argv);

    auto resource_manager = resource::make_relative_manager();
    gpu::DeviceKind backend = gpu::DeviceKind::unknown;
    if (cmdline.hasSwitch("metal")) {
        backend = gpu::DeviceKind::metal;
    }
    if (cmdline.hasSwitch("vulkan")) {
        backend = gpu::DeviceKind::vulkan;
    }
    auto device = gpu::makeDevice(backend);

    auto buf0 = device->createBuffer(gpu::BufferUsage::uniform,
                                     base::make_span<u8>(sizeof(count), (u8*)&count));
    auto buf1 = device->createBuffer(gpu::BufferUsage::storage,
                                     base::make_span<u8>(sizeof(arr1), (u8*)&arr1));
    auto buf2 = device->createBuffer(gpu::BufferUsage::storage,
                                     base::make_span<u8>(sizeof(arr2), (u8*)&arr2));
    auto buf3 =
        device->createBuffer(gpu::BufferUsage::storage | gpu::BufferUsage::map_read, sizeof(arr1));

    resource::directory_resource* blob = nullptr;

    if (backend == gpu::DeviceKind::metal) {
        blob = resource_manager->open(resource::resource_id("shader.metallib"));
    } else if (backend == gpu::DeviceKind::vulkan) {
        blob = resource_manager->open(resource::resource_id("demo_sum.spirv"));
    }
    auto library = device->createShaderLibrary(blob->getSpan());

    gpu::BindGroupLayoutBuilder builder;
    builder.setStage(gpu::ShaderStage::compute);
    builder.addEntry(0, gpu::BindEntryKind::uniform_buffer);  // control
    builder.addEntry(1, gpu::BindEntryKind::storage_buffer);  // in1
    builder.addEntry(2, gpu::BindEntryKind::storage_buffer);  // in2
    builder.addEntry(3, gpu::BindEntryKind::storage_buffer);  // result
    auto layout = builder.build(device.get());

    auto pipeline = device->createComputePipeline({library, "add_arrays"}, {layout});
    // auto max_size = pipeline->maxGroupSize();
    // spargel_log_info("maxGroupSize = %u", max_size);

    auto queue = device->createCommandQueue();

    auto cmdbuf = queue->createCommandBuffer();
    auto encoder = cmdbuf->beginComputePass();
    encoder->setComputePipeline(pipeline);
    spargel_log_info("pipeline set!");
    encoder->setBuffer(buf0, {.apple = {.buffer_index = 0}});
    encoder->setBuffer(buf1, {.apple = {.buffer_index = 1}});
    encoder->setBuffer(buf2, {.apple = {.buffer_index = 2}});
    encoder->setBuffer(buf3, {.apple = {.buffer_index = 3}});
    encoder->dispatch({.x = 1, .y = 1, .z = 1}, {.x = 64, .y = 1, .z = 1});
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
