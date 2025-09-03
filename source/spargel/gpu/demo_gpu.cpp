// #include "spargel/codec/json.h"
#include "spargel/gpu/gpu.h"

namespace spargel::gpu {
    namespace {
        void demoMain() {
            // auto json = codec::parseJson(R"(
            //     {
            //         "id": 0,
            //         "kind": "buffer"
            //     }
            // )");
            // codec::JsonDecodeBackend backend;
            // auto entry = BindTableEntry::CODEC.decode(backend, base::move(json.left())).left();

            auto device = Device::create();
            auto shader_library = device->createShaderLibrary({});
            auto _ = device->createComputePipeline2({
                .compute_stage =
                    PipelineStage{
                        .shader =
                            ShaderFunction{
                                .library = shader_library,
                                .entry = "main",
                            },
                        .groups =
                            {
                                PipelineArgumentGroup{
                                    .location = 0,
                                    .arguments =
                                        {
                                            BindGroupEntry{
                                                .id = 0,
                                                .kind = BindEntryKind::uniform_buffer,
                                            },
                                        },
                                },
                            },
                    },
            });
        }
    }  // namespace
}  // namespace spargel::gpu

int main() {
    spargel::gpu::demoMain();
    return 0;
}
