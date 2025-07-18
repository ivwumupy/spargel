#pragma once

#include <spargel/base/string.h>
#include <spargel/text/font.h>

// FreeType 2
#include <ft2build.h>
#include FT_FREETYPE_H

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
        FontFreeType(FT_Library library, FT_Face face);
        ~FontFreeType();

        base::StringView name() override { return name_.view(); }
        Bitmap rasterGlyph(GlyphId id, float scale) override;
        GlyphInfo glyphInfo(GlyphId id) override;

    private:
        friend class TextShaperHarfBuzz;

        FT_Library library;
        FT_Face face;

        base::String name_;

        void loadGlyph(u32 id);
    };

}  // namespace spargel::text
