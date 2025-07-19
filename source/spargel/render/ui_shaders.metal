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
    CMD_SET_CLIP,
    CMD_CLEAR_CLIP,
    CMD_SAMPLE_TEXTURE,
    CMD_DUMP,
};

#define ATLAS_SIZE 2048

[[fragment]]
float4 sdf_frag(
    Sdf_VOut in [[stage_in]],
    constant UniformData const& uniform [[buffer(0)]],
    constant uint8_t const* cmds [[buffer(1)]],
    constant float const* data [[buffer(2)]],
    texture2d<float> glyph_texture [[texture(0)]]
) {
    constexpr sampler texture_sampler(mag_filter::linear, min_filter::linear);

    float2 p = in.position.xy;
    uint j = 0;
    float4 col = float4(0, 0, 0, 1);
    bool has_clip = false;
    float4 clip = float4(0, 0, 0, 0);

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
        } else if (cmd == CMD_SET_CLIP) {
            clip = float4(data[j], data[j+1], data[j+2], data[j+3]);
            has_clip = true;
            j += 4;
        } else if (cmd == CMD_CLEAR_CLIP) {
            has_clip = false;
        } else if (cmd == CMD_SAMPLE_TEXTURE) {
            float2 origin = float2(data[j], data[j+1]);
            float2 size = float2(data[j+2], data[j+3]);
            float2 tex_uv0 = float2(as_type<ushort2>(data[j+4])) / ATLAS_SIZE;
            float2 tex_uvs = float2(as_type<ushort2>(data[j+5])) / ATLAS_SIZE;
            c1 = float4(as_type<uchar4>(data[j+6])) / 255.0;
            j += 7;

            if (p.x >= origin.x && p.y >= origin.y && p.x <= (origin.x + size.x) && p.y <= (origin.y + size.y)) {
                float2 dp = p - origin;
                float2 uv = (dp / size) * tex_uvs + tex_uv0;
                c1.a *= glyph_texture.sample(texture_sampler, uv).a;
                d = 0.0;
            }
        } else if (cmd == CMD_DUMP) {
            // TODO:
            float2 uv = p / 1000;
            c1.a *= glyph_texture.sample(texture_sampler, uv).a;
            d = 0.0;
        } else {
            // do nothing
        }
        if (has_clip && p.x >= clip.x && p.y >= clip.y && p.x <= (clip.x + clip.z) && p.y <= (clip.y + clip.w))
            continue;
        // TODO: The mixing model is not good.
        // col = mix(col, c1, clamp(1.0 - d, 0.0, 1.0));
        col.xyz = mix(col.xyz, c1.xyz, clamp(1.0 - d, 0.0, 1.0) * c1.a);
    }
    //return col;
    return float4(col.xyz, 1.0);
}

[[kernel]]
void sdf_comp(
    ushort2 tid [[thread_position_in_grid]],
    constant UniformData const& uniform [[buffer(0)]],
    constant uint8_t const* cmds [[buffer(1)]],
    constant float const* data [[buffer(2)]],
    texture2d<float, access::write> target [[texture(0)]],
    texture2d<float> glyph_texture [[texture(1)]]
) {
    constexpr sampler texture_sampler(mag_filter::linear, min_filter::linear);

    if (tid.x >= target.get_width() || tid.y >= target.get_height()) {
        return;
    }

    float2 p = float2(tid);
    uint j = 0;
    float4 col = float4(0, 0, 0, 1);
    bool has_clip = false;
    float4 clip = float4(0, 0, 0, 0);

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
        } else if (cmd == CMD_SET_CLIP) {
            clip = float4(data[j], data[j+1], data[j+2], data[j+3]);
            has_clip = true;
            j += 4;
        } else if (cmd == CMD_CLEAR_CLIP) {
            has_clip = false;
        } else if (cmd == CMD_SAMPLE_TEXTURE) {
            float2 origin = float2(data[j], data[j+1]);
            float2 size = float2(data[j+2], data[j+3]);
            float2 tex_uv0 = float2(as_type<ushort2>(data[j+4])) / ATLAS_SIZE;
            float2 tex_uvs = float2(as_type<ushort2>(data[j+5])) / ATLAS_SIZE;
            c1 = float4(as_type<uchar4>(data[j+6])) / 255.0;
            j += 7;

            if (p.x >= origin.x && p.y >= origin.y && p.x <= (origin.x + size.x) && p.y <= (origin.y + size.y)) {
                float2 dp = p - origin;
                float2 uv = (dp / size) * tex_uvs + tex_uv0;
                c1.a *= glyph_texture.sample(texture_sampler, uv).a;
                d = 0.0;
            }
        } else if (cmd == CMD_DUMP) {
            // TODO:
            float2 uv = p / 1000;
            c1.a *= glyph_texture.sample(texture_sampler, uv).a;
            d = 0.0;
        } else {
            // do nothing
        }
        if (has_clip && p.x >= clip.x && p.y >= clip.y && p.x <= (clip.x + clip.z) && p.y <= (clip.y + clip.w))
            continue;
        // TODO: The mixing model is not good.
        // col = mix(col, c1, clamp(1.0 - d, 0.0, 1.0));
        col.xyz = mix(col.xyz, c1.xyz, clamp(1.0 - d, 0.0, 1.0) * c1.a);
    }

    target.write(float4(col.rgb, 1.0), tid);
}
