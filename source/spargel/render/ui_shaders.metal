#include <metal_stdlib>
using namespace metal;

struct UniformData {
    uint cmd_count;
};

struct Sdf_VOut {
    float4 position [[position]];
};

constant float4 VERTICES[] = {
    {-1.0,  1.0, 0.0, 1.0},
    {-1.0, -3.0, 0.0, 1.0},
    { 3.0,  1.0, 0.0, 1.0},
};

[[vertex]]
float4 sdf_vert(uint vid [[vertex_id]]) {
    return VERTICES[vid];
}

float sdf_circle_fill(float2 p, float r) {
    return length(p) - r;
}

float sdf_circle_stroke(float2 p, float r) {
    return abs(length(p) - r);
}

float sdf_segment(float2 p, float2 a, float2 b) {
    float2 ap = p - a;
    float2 ab = b - a;
    float t = clamp(dot(ap, ab) / dot(ab, ab), 0.0, 1.0);
    return length(ap - ab * t);
}

enum {
    CMD_FILL_RECT = 0,
    CMD_FILL_CIRCLE,
    CMD_STROKE_SEGMENT,
    CMD_STROKE_CIRCLE,
};

[[fragment]]
float4 sdf_frag(
    Sdf_VOut in [[stage_in]],
    constant UniformData const& uniform [[buffer(0)]],
    constant uint8_t const* cmds [[buffer(1)]],
    constant float const* data [[buffer(2)]]
) {
    float2 p = in.position.xy;
    uint j = 0;
    float4 col = float4(0, 0, 0, 1);
    for (uint i = 0; i < uniform.cmd_count; i++) {
        uint8_t cmd = cmds[i];
        float d = 1.0;
        float4 c1 = float4(1, 0, 1, 1);
        if (cmd == CMD_FILL_CIRCLE) {
            float2 center = float2(data[j], data[j+1]);
            float radius = data[j+2];
            c1 = float4(as_type<uchar4>(data[j+3])) / 255.0;
            d = sdf_circle_fill(p - center, radius);
            j += 4;
        } else if (cmd == CMD_STROKE_SEGMENT) {
            float2 start = float2(data[j], data[j+1]);
            float2 end = float2(data[j+2], data[j+3]);
            c1 = float4(as_type<uchar4>(data[j+4])) / 255.0;
            d = sdf_segment(p, start, end);
            j += 5;
        } else if (cmd == CMD_STROKE_CIRCLE) {
            float2 center = float2(data[j], data[j+1]);
            float radius = data[j+2];
            c1 = float4(as_type<uchar4>(data[j+3])) / 255.0;
            d = sdf_circle_stroke(p - center, radius);
            j += 4;
        } else {
            // do nothing
        }
        col = mix(col, c1, clamp(1.0 - d, 0.0, 1.0));
    }
    return col;
}
