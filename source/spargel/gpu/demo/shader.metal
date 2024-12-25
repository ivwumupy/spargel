#include <metal_stdlib>
using namespace metal;

struct QuadData {
    float2 origin;
    float2 size;
    float2 cell_origin;
};

struct VertexData {
    float2 position [[attribute(0)]];
};

struct RasterData {
    float4 position [[position]];
    float2 texture_coord;
};

struct UniformData {
    float2 viewport;
};

[[vertex]] RasterData vertex_shader(uint vertex_id [[vertex_id]],
                                    uint instance_id [[instance_id]],
                                    VertexData in [[stage_in]],
                                    constant QuadData* quads [[buffer(1)]],
                                    constant UniformData& uniform [[buffer(2)]]) {
    float2 pixel_position = in.position * quads[instance_id].size + quads[instance_id].origin;

    RasterData out;
    out.position = float4(0.0, 0.0, 0.0, 1.0);
    out.position.xy = pixel_position / (uniform.viewport / 2.0);
    out.texture_coord = float2(in.position.x, 1.0 - in.position.y) * quads[instance_id].size + quads[instance_id].cell_origin;
    out.texture_coord /= float2(512, 512);
    return out;
}

[[fragment]] float4 fragment_shader(RasterData in [[stage_in]],
                                    texture2d<float> color_texture [[texture(0)]]) {
    constexpr sampler texture_sampler(mag_filter::linear, min_filter::linear);
    float4 color = float4(1.0, 0.0, 0.0, 1.0);
    color.a *= color_texture.sample(texture_sampler, in.texture_coord).a;
    return color;
}

[[kernel]] void add_arrays(constant float const* inA [[buffer(0)]],
                           constant float const* inB [[buffer(1)]],
                           device float* result [[buffer(2)]],
                           uint index [[thread_position_in_grid]]) {
    result[index] = inA[index] + inB[index];
}
