#include "spargel/codec/json.h"
#include "spargel/gpu/gpu.h"

namespace spargel::gpu {
    namespace {
        void demoMain() {
            auto json = codec::parseJson(R"XXX(
                {
                    "id": 0,
                    "kind": "buffer"
                }
            )XXX");
            codec::JsonDecodeBackend backend;
            auto entry = BindTableEntry::CODEC.decode(backend, base::move(json.left())).left();

            spargel_log_info("id: %d", entry.id);

            // auto device = Device::create();
            // auto shader_library = device->createShaderLibrary({});
            // auto pipeline = device->createComputePipeline2({
            //     .compute = {
            //         .library = shader_library,
            //         .entry = "main",
            //     },
            //     .groups = {
            //         PipelineArgumentGroup {
            //             .stage = ShaderStage::compute,
            //         },
            //     },
            // });
        }
    }  // namespace
}  // namespace spargel::gpu

int main() {
    spargel::gpu::demoMain();
    return 0;
}
