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

// centered at origin, s is the size.
float sdf_rrect(float2 p, float2 s, float r) {
    float2 q = abs(p) - (s / 2.0 - r);
    return min(max(q.x, q.y), 0.0f) + length(max(q, 0.0f)) - r;
}

enum {
    CMD_FILL_RECT = 0,
    CMD_FILL_CIRCLE,
    CMD_FILL_ROUNDED_RECT,
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
        if (has_clip && p.x >= clip.x && p.y >= clip.y && p.x <= (clip.x + clip.z) && p.y <= (clip.y + clip.w)) {
            // TODO: The mixing model is not good.
            // col = mix(col, c1, clamp(1.0 - d, 0.0, 1.0));
            col.xyz = mix(col.xyz, c1.xyz, clamp(1.0 - d, 0.0, 1.0) * c1.a);
        }
    }
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

    float2 p = float2(tid) + 0.5;
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
        if (has_clip && p.x >= clip.x && p.y >= clip.y && p.x <= (clip.x + clip.z) && p.y <= (clip.y + clip.w)) {
            // TODO: The mixing model is not good.
            // col = mix(col, c1, clamp(1.0 - d, 0.0, 1.0));
            col.xyz = mix(col.xyz, c1.xyz, clamp(1.0 - d, 0.0, 1.0) * c1.a);
        }
    }

    target.write(float4(col.rgb, 1.0), tid);
}

struct Command2 {
    uchar cmd;
    float4 clip;
    float data[8];
};
static_assert(sizeof(Command2) == 64, "size does not match");

// One command for the tile.
struct BinSlot {
    // Index of the command in the command buffer.
    uint command_index;
    // Index of the next slot.
    uint next_slot;
};

static_assert(sizeof(BinSlot) == 8, "size error");

struct BinControl {
    uint tile_count_x;
    uint tile_count_y;
    uint cmd_count;
    // The capacity of the BinSlot buffer.
    uint max_slot;
};

struct BinAlloc {
    // Set to zero externally.
    // NOTE: Offset keeps increasing even if the buffer is out of space.
    atomic_uint offset;
    bool out_of_space;
};

static_assert(sizeof(BinAlloc) == 8, "size error");

[[kernel]]
void sdf_comp_v2(
    ushort2 tid [[thread_position_in_grid]],
    constant BinControl const& uniform [[buffer(0)]],
    constant Command2 const* cmds [[buffer(1)]],
    constant BinSlot const* slots [[buffer(2)]],
    texture2d<float, access::write> target [[texture(0)]],
    texture2d<float> glyph_texture [[texture(1)]]
) {
    constexpr sampler texture_sampler(mag_filter::linear, min_filter::linear);

    if (tid.x >= target.get_width() || tid.y >= target.get_height()) {
        return;
    }

    uint2 tile = uint2(tid) / 8;
    uint slot_id = tile.x * uniform.tile_count_y + tile.y;

    float2 p = float2(tid) + 0.5;
    float4 col = float4(0, 0, 0, 1);
    float4 clip = float4(0, 0, 0, 0);

    //for (uint i = 0; i < uniform.cmd_count; i++) {
    while (true) {
        threadgroup_barrier(mem_flags::mem_none);
        
        uint cmd_id = slots[slot_id].command_index;
        uint next_slot = slots[slot_id].next_slot;

        if (cmd_id >= uniform.cmd_count || next_slot >= uniform.max_slot) {
            break;
        }

        slot_id = next_slot;

        // TODO: Put command in the threadgroup memory.
        Command2 cmd = cmds[cmd_id];

        float d = 1.0;
        float4 c1 = float4(1, 0, 1, 1);
        clip = cmd.clip;

        if (cmd.cmd == CMD_FILL_CIRCLE) {
            float2 center = float2(cmd.data[0], cmd.data[1]);
            float radius = cmd.data[2];
            c1 = float4(as_type<uchar4>(cmd.data[3])) / 255.0;
            d = sdf_circle_fill(p - center, radius);
        } else if (cmd.cmd == CMD_STROKE_SEGMENT) {
            float2 start = float2(cmd.data[0], cmd.data[1]);
            float2 end = float2(cmd.data[2], cmd.data[3]);
            c1 = float4(as_type<uchar4>(cmd.data[4])) / 255.0;
            d = sdf_segment(p, start, end);
        } else if (cmd.cmd == CMD_STROKE_CIRCLE) {
            float2 center = float2(cmd.data[0], cmd.data[1]);
            float radius = cmd.data[2];
            c1 = float4(as_type<uchar4>(cmd.data[3])) / 255.0;
            d = sdf_circle_stroke(p - center, radius);
        } else if (cmd.cmd == CMD_SAMPLE_TEXTURE) {
            float2 origin = float2(cmd.data[0], cmd.data[1]);
            float2 size = float2(cmd.data[2], cmd.data[3]);
            float2 tex_uv0 = float2(as_type<ushort2>(cmd.data[4])) / ATLAS_SIZE;
            float2 tex_uvs = float2(as_type<ushort2>(cmd.data[5])) / ATLAS_SIZE;
            c1 = float4(as_type<uchar4>(cmd.data[6])) / 255.0;

            if (p.x >= origin.x && p.y >= origin.y && p.x <= (origin.x + size.x) && p.y <= (origin.y + size.y)) {
                float2 dp = p - origin;
                float2 uv = (dp / size) * tex_uvs + tex_uv0;
                c1.a *= glyph_texture.sample(texture_sampler, uv).a;
                d = 0.0;
            }
        } else if (cmd.cmd == CMD_DUMP) {
            // TODO:
            float2 uv = p / 1000;
            c1.a *= glyph_texture.sample(texture_sampler, uv).a;
            d = 0.0;
        } else {
            // do nothing
        }
        if (p.x >= clip.x && p.y >= clip.y && p.x <= (clip.x + clip.z) && p.y <= (clip.y + clip.w)) {
            // TODO: The mixing model is not good.
            // col = mix(col, c1, clamp(1.0 - d, 0.0, 1.0));
            col.xyz = mix(col.xyz, c1.xyz, clamp(1.0 - d, 0.0, 1.0) * c1.a);
        }
    }

    target.write(float4(col.rgb, 1.0), tid);
}

bool bboxIntersect(float4 a, float4 b) {
    return (abs((a.x + a.z / 2) - (b.x + b.z / 2)) * 2 < (a.z + b.z)) &&
           (abs((a.y + a.w / 2) - (b.y + b.w / 2)) * 2 < (a.w + b.w));
}

// Each thread computes the commands of a 8x8 tile.
//
// The first slot is reserved.
[[kernel]]
void sdf_binning(
    ushort2 tid [[thread_position_in_grid]],
    // ushort2 grid_size [[threads_per_grid]],
    constant BinControl const& uniform [[buffer(0)]],
    constant Command2 const* cmds [[buffer(1)]],
    device BinSlot* slots [[buffer(2)]],
    device BinAlloc& alloc [[buffer(3)]]
) {
    if (tid.x >= uniform.tile_count_x || tid.y >= uniform.tile_count_y) {
        return;
    }

    float4 tile;
    tile.xy = float2(tid) * 8.0;
    tile.zw = 8.0;

    uint slot_id = tid.x * uniform.tile_count_y + tid.y;

    for (uint i = 0; i < uniform.cmd_count; i++) {
        threadgroup_barrier(mem_flags::mem_none);

        Command2 cmd = cmds[i];

        // Compute the bounding box of the command.

        float4 bbox = 0;
        if (cmd.cmd == CMD_FILL_CIRCLE) {
            float2 center = float2(cmd.data[0], cmd.data[1]);
            float radius = cmd.data[2];
            radius += 0.5;
            bbox.xy = center - radius;
            bbox.zw = float2(radius) * 2;
        } else if (cmd.cmd == CMD_FILL_ROUNDED_RECT) {
            float2 origin = float2(cmd.data[0], cmd.data[1]);
            float2 size = float2(cmd.data[2], cmd.data[3]);
            bbox.xy = origin - 0.5;
            bbox.zw = size + 1;
        } else if (cmd.cmd == CMD_STROKE_SEGMENT) {
            float2 start = float2(cmd.data[0], cmd.data[1]);
            float2 end = float2(cmd.data[2], cmd.data[3]);
            bbox.xy = min(start, end) - 0.5;
            bbox.zw = abs(end - start) + 1;
        } else if (cmd.cmd == CMD_STROKE_CIRCLE) {
            float2 center = float2(cmd.data[0], cmd.data[1]);
            float radius = cmd.data[2];
            radius += 0.5;
            bbox.xy = center - radius;
            bbox.zw = float2(radius) * 2;
        } else if (cmd.cmd == CMD_SAMPLE_TEXTURE) {
            float2 origin = float2(cmd.data[0], cmd.data[1]);
            float2 size = float2(cmd.data[2], cmd.data[3]);
            bbox.xy = origin - 0.5;
            bbox.zw = size + 1;
        } else if (cmd.cmd == CMD_DUMP) {
        } else {
            // do nothing
        }

        // Check intersection.
        if (!bboxIntersect(tile, bbox)) {
            continue;
        }

        // Allocate a new slot.

        uint new_slot = atomic_fetch_add_explicit(&alloc.offset, 1, memory_order_relaxed);
        
        if (new_slot >= uniform.max_slot) {
            alloc.out_of_space = true;
            continue;
        }

        slots[slot_id].next_slot = new_slot;
        slots[slot_id].command_index = i;
        slot_id = new_slot;
    }
    slots[slot_id].next_slot = -1;
    slots[slot_id].command_index = -1;
}

[[fragment]]
float4 sdf_frag2(
    Sdf_VOut in [[stage_in]],
    constant BinControl const& uniform [[buffer(0)]],
    constant Command2 const* cmds [[buffer(1)]],
    constant BinSlot* slots [[buffer(2)]],
    texture2d<float> glyph_texture [[texture(0)]]
) {
    //constexpr sampler texture_sampler(mag_filter::linear, min_filter::linear);
    constexpr sampler texture_sampler;

    float2 p = in.position.xy;
    float4 col = float4(0, 0, 0, 1);
    float4 clip = float4(0, 0, 0, 0);

    uint2 tile = uint2(p) / 8;
    uint slot_id = tile.x * uniform.tile_count_y + tile.y;

    //for (uint i = 0; i < uniform.cmd_count; i++) {
    //    uint8_t cmd = cmds[i];
    while (true) {
        threadgroup_barrier(mem_flags::mem_none);
        
        uint cmd_id = slots[slot_id].command_index;
        uint next_slot = slots[slot_id].next_slot;

        if (cmd_id >= uniform.cmd_count || next_slot >= uniform.max_slot) {
            break;
        }

        slot_id = next_slot;

        // TODO: Put command in the threadgroup memory.
        Command2 cmd = cmds[cmd_id];

        float d = 1.0;
        float4 c1 = float4(1, 0, 1, 1);
        clip = cmd.clip;

        if (cmd.cmd == CMD_FILL_CIRCLE) {
            float2 center = float2(cmd.data[0], cmd.data[1]);
            float radius = cmd.data[2];
            c1 = float4(as_type<uchar4>(cmd.data[3])) / 255.0;
            d = sdf_circle_fill(p - center, radius);
        } else if (cmd.cmd == CMD_FILL_ROUNDED_RECT) {
            float2 origin = float2(cmd.data[0], cmd.data[1]);
            float2 size = float2(cmd.data[2], cmd.data[3]);
            float radius = cmd.data[4];
            c1 = float4(as_type<uchar4>(cmd.data[5])) / 255.0;
            d = sdf_rrect(p - origin - size / 2.0, size, radius);
        } else if (cmd.cmd == CMD_STROKE_SEGMENT) {
            float2 start = float2(cmd.data[0], cmd.data[1]);
            float2 end = float2(cmd.data[2], cmd.data[3]);
            c1 = float4(as_type<uchar4>(cmd.data[4])) / 255.0;
            d = sdf_segment(p, start, end);
        } else if (cmd.cmd == CMD_STROKE_CIRCLE) {
            float2 center = float2(cmd.data[0], cmd.data[1]);
            float radius = cmd.data[2];
            c1 = float4(as_type<uchar4>(cmd.data[3])) / 255.0;
            d = sdf_circle_stroke(p - center, radius);
        } else if (cmd.cmd == CMD_SAMPLE_TEXTURE) {
            float2 origin = float2(cmd.data[0], cmd.data[1]);
            float2 size = float2(cmd.data[2], cmd.data[3]);
            float2 tex_uv0 = float2(as_type<ushort2>(cmd.data[4])) / ATLAS_SIZE;
            float2 tex_uvs = float2(as_type<ushort2>(cmd.data[5])) / ATLAS_SIZE;
            c1 = float4(as_type<uchar4>(cmd.data[6])) / 255.0;

            if (p.x >= origin.x && p.y >= origin.y && p.x <= (origin.x + size.x) && p.y <= (origin.y + size.y)) {
                float2 dp = p - origin;
                float2 uv = (dp / size) * tex_uvs + tex_uv0;
                c1.a *= glyph_texture.sample(texture_sampler, uv).a;
                d = 0.0;
            }
        } else if (cmd.cmd == CMD_DUMP) {
            // TODO:
            float2 uv = p / 1000;
            c1.a *= glyph_texture.sample(texture_sampler, uv).a;
            d = 0.0;
        } else {
            // do nothing
        }
        if (p.x >= clip.x && p.y >= clip.y && p.x <= (clip.x + clip.z) && p.y <= (clip.y + clip.w)) {
            // TODO: The mixing model is not good.
            // col = mix(col, c1, clamp(1.0 - d, 0.0, 1.0));
            col.xyz = mix(col.xyz, c1.xyz, clamp(1.0 - d, 0.0, 1.0) * c1.a);
        }
    }
    return float4(col.xyz, 1.0);
}

