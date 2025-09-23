#include "spargel/codec/json_codec.h"
#include "spargel/gpu/gpu.h"
#include "spargel/json/json_parser.h"

namespace spargel::gpu {
    namespace {
        void demoMain() {
            // auto json = json::parseJson(R"(
            //     {
            //         "functions": [
            //             {
            //                 "shader_id": "/shaders/ui_render/binning.comp",
            //                 "entry_name": "ui_binning"
            //             },
            //             {
            //                 "shader_id": "/shaders/ui_render/dummy.vert",
            //                 "entry_name": "ui_vert"
            //             },
            //             {
            //                 "shader_id":
            //                 "/shaders/ui_render/list_raster.frag",
            //                 "entry_name": "ui_frag"
            //             }
            //         ]
            //     }
            //)");
            // codec::JsonDecodeBackend backend;
            // auto entry = MetalLibraryMeta::CODEC.decode(
            //     backend, base::move(json.left()));

            // spargel_check(entry.isLeft());
            //  entry.right().dump();

            auto device = Device::create();
            device->createShaderFunction("/shaders/demo/triangle_vert");
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
            //                                     .kind =
            //                                     BindEntryKind::uniform_buffer,
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
