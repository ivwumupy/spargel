#pragma once

#include <spargel/base/hash.h>
#include <spargel/base/string_view.h>
#include <spargel/base/types.h>
#include <spargel/base/unique_ptr.h>
#include <spargel/base/vector.h>
#include <spargel/math/rectangle.h>
#include <spargel/math/vector.h>

namespace spargel::text {
    struct GlyphId {
        u32 value;
        friend bool operator==(GlyphId lhs, GlyphId rhs) { return lhs.value == rhs.value; }
        friend void tag_invoke(base::tag<base::hash>, base::HashRun& run, GlyphId const& self) {
            run.combine(self.value);
        }
    };
    struct Bitmap {
        usize width;
        usize height;
        base::Vector<base::Byte> data;
    };
    //  | bounding box
    //  |  +--------+      - ascent
    //  |  |        |      |
    //  |  |        |      |
    // -*--+--------+--*---+--->
    //  |  |        |  +---+- horizontal advance
    //  |  +--------+      - descent
    struct GlyphInfo {
        // The bounding box of the glyph.
        math::Rectangle bounding_box;
        // The origin of the next glyph.
        float horizontal_advance;
        // The (signed) distance to the top of the bounding box.
        float ascent() const { return bounding_box.origin.y; }
        // The (signed) distance to the bottom of the bounding box.
        float descent() const { return bounding_box.origin.y - bounding_box.size.height; }
        float width() const { return bounding_box.size.width; }
        float height() const { return bounding_box.size.height; }
    };
    class Font {
    public:
        virtual ~Font() = default;

        virtual base::StringView name() = 0;

        // TODO
        virtual Bitmap rasterGlyph(GlyphId id, float scale) = 0;

        virtual GlyphInfo glyphInfo(GlyphId id) = 0;
    };
    base::UniquePtr<Font> createDefaultFont();
}  // namespace spargel::text
