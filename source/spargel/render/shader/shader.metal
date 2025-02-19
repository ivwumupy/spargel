#include <metal_stdlib>
using namespace metal;

struct VertexData {
    float3 position [[attribute(0)]];
};

struct RasterData {
    float4 position [[position]];
    float3 c;
};

struct UniformData {
    float4x4 model_to_world;
    float4x4 world_to_camera;
    float4x4 camera_to_clip;
};

// The bunny mesh is too small.
constant float model_scale = 1000;

[[vertex]] RasterData vertex_shader(VertexData in [[stage_in]], // buffer 0
                                    constant UniformData& uniform [[buffer(1)]],
                                    uint vid [[vertex_id]]) {
    float3 model_position = in.position * model_scale;
    float4 world_position = uniform.model_to_world * float4(model_position, 1.0);
    float4 camera_position = uniform.world_to_camera * world_position;
    float4 clip_position = uniform.camera_to_clip * camera_position;

    RasterData out;
    out.position = clip_position;

    float3 cs[3] = {
        {1.0, 0.0, 0.0},
        {0.0, 1.0, 0.0},
        {0.0, 0.0, 1.0},
    };

    out.c = cs[vid % 3];
    return out;
}

[[fragment]] float4 fragment_shader(RasterData in [[stage_in]]) {
    float4 color = float4(in.c, 1.0);
    return color;
}
