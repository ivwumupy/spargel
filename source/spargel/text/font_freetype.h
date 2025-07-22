#pragma once

#include <spargel/base/string.h>
#include <spargel/text/font.h>

// FreeType 2
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SIZES_H

namespace spargel::text {

    class FreeTypeLibrary {
    public:
        FreeTypeLibrary() { FT_Init_FreeType(&library); }
        ~FreeTypeLibrary() { FT_Done_FreeType(library); }

        FT_Library library;
    };

    extern FreeTypeLibrary defaultFreeTypeLibrary;

    class FontFreeType : public Font {
    public:
        FontFreeType(FT_Library library, FT_Face face, float size);
        ~FontFreeType();

        base::StringView name() override { return name_.view(); }
        Bitmap rasterGlyphXY(GlyphId id, float scale_x, float scale_y);
        Bitmap rasterGlyph(GlyphId id, float scale) override;
        GlyphInfo glyphInfo(GlyphId id) override;

    private:
        friend class TextShaperHarfBuzz;

        FT_Library library;
        FT_Face face;

        float size;
        base::String name_;

        void loadGlyph(u32 id, u32 load_flags = FT_LOAD_DEFAULT);
    };

}  // namespace spargel::text
