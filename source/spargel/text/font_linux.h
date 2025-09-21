#include "spargel/base/string.h"
#include "spargel/text/font.h"
#include "spargel/text/font_manager_linux.h"

#if SPARGEL_ENABLE_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

#if SPARGEL_ENABLE_HARFBUZZ
#include <harfbuzz/hb.h>
#endif

namespace spargel::text {

    class FontLinux : public Font {
    public:
        FontLinux(FontManagerLinux* font_manager, base::Span<u8> buffer,
                  float size);

        base::StringView name() override { return name_.view(); }

        Bitmap rasterizeGlyph(GlyphId id, float scale) override;

        GlyphInfo glyphInfo(GlyphId id) override;

    private:
        friend class TextShaperLinux;
        FontManagerLinux* font_manager;
        base::Span<u8> buffer;
        float size;

        base::String name_;

        u16 units_per_EM;

#if SPARGEL_ENABLE_FREETYPE
        FT_Face ft_face = nullptr;
        float ft_last_scale = 0;

        void initFreeType();
        void doneFreeType();
#endif

#if SPARGEL_ENABLE_HARFBUZZ
        hb_blob_t* hb_blob;
        hb_face_t* hb_face;
        hb_font_t* hb_font = nullptr;

        void initHarfBuzz();
        void doneHarfBuzz();
#endif
    };

}  // namespace spargel::text
