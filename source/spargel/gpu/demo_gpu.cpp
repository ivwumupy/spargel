#include "spargel/codec/json_codec.h"
#include "spargel/gpu/gpu.h"
#include "spargel/json/json_parser.h"

namespace spargel::gpu {
    namespace {
        void demoMain() {
            auto json = json::parseJson(R"(
                {
                    "id": 0,
                    "kind": 123
                }
            )");
            codec::JsonDecodeBackend backend;
            auto entry = BindTableEntry::CODEC.decode(backend, base::move(json.left()));

            spargel_check(entry.isRight());
            entry.right().dump();

            // auto device = Device::create();
            // auto shader_library = device->createShaderLibrary({});
            // auto _ = device->createComputePipeline2({
            //     .compute_stage =
            //         PipelineStage{
            //             .shader =
            //                 ShaderFunction{
            //                     .library = shader_library,
            //                     .entry = "main",
            //                 },
            //             .groups =
            //                 {
            //                     PipelineArgumentGroup{
            //                         .location = 0,
            //                         .arguments =
            //                             {
            //                                 BindGroupEntry{
            //                                     .id = 0,
            //                                     .kind = BindEntryKind::uniform_buffer,
            //                                 },
            //                             },
            //                     },
            //                 },
            //         },
            // });
        }
    }  // namespace
}  // namespace spargel::gpu

int main() {
    spargel::gpu::demoMain();
    return 0;
}
