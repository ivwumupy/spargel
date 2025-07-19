#pragma once

#include <spargel/base/string.h>
#include <spargel/text/font.h>

//
#include <CoreText/CoreText.h>

namespace spargel::text {
    class FontMac final : public Font {
    public:
        FontMac(CTFontRef object);
        ~FontMac();

        base::StringView name() override { return name_.view(); }
        Bitmap rasterGlyph(GlyphId id, float scale) override;
        Bitmap rasterGlyph(GlyphId id, float scale, math::Vector2f subpixel_position) override;
        GlyphInfo glyphInfo(GlyphId id) override;

    private:
        friend class TextShaperMac;

        CTFontRef object_;
        base::String name_;
    };
}  // namespace spargel::text
