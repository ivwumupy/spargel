#pragma once

#include <spargel/base/types.h>
#include <spargel/base/vector.h>

namespace spargel::text {
    struct GlyphId {
        u32 value;
    };
    struct Bitmap {
        usize width;
        usize height;
        base::Vector<base::Byte> data;
    };
    struct GlyphInfo {
        float descent;
        float ascent;
    };
    class Font {
    public:
        virtual ~Font() = default;

        // TODO
        virtual Bitmap rasterGlyph(GlyphId id) = 0;

        virtual void glyphInfo(GlyphId id) = 0;
    };
}  // namespace spargel::text
