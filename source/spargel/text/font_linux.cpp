#include "spargel/text/font_linux.h"

#include <string.h>

#include "font.h"

namespace {
    const int points_per_inch = 72;
}

namespace spargel::text {

    FontLinux::FontLinux(FontManagerLinux* font_manager, base::Span<u8> buffer, float size)
        : font_manager(font_manager), buffer(buffer), size(size) {
        spargel_check(font_manager);
        spargel_check(size > 0);

#if SPARGEL_ENABLE_FREETYPE
        initFreeType();
        {
            char *family_name = ft_face->family_name, *style_name = ft_face->style_name;
            base::String name(family_name ? family_name : "<unknown>");
            if (style_name) {
                name = name + " (" + style_name + ')';
            }
            name_ = base::move(name);
        }
#else
#error "unimplemented"
#endif
    }

    Bitmap FontLinux::rasterizeGlyph(GlyphId id, float scale) {
        if (!ft_face) initFreeType();

        if (scale != ft_last_scale) {
            if (FT_Set_Char_Size(ft_face, (u32)math::round(size * 64), (u32)math::round(size * 64),
                                 (u32)math::round(scale * points_per_inch),
                                 (u32)math::round(scale * points_per_inch)) != 0) {
                spargel_log_fatal("Error calling FT_Set_Char_Size.");
                spargel_panic_here();
            }
            ft_last_scale = scale;
        }

        u32 glyph_index = id.value;

        if (FT_Load_Glyph(ft_face, glyph_index, FT_LOAD_DEFAULT) != 0) {
            spargel_log_fatal("Error calling FT_Load_Glyph (glyph_index=%d).", glyph_index);
            spargel_panic_here();
        }

        auto& slot = ft_face->glyph;
        if (FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL) != 0) {
            spargel_log_fatal("Error calling FT_Render_Glyph (glyph_index=%d).", glyph_index);
            spargel_panic_here();
        }

        // FIXME
        spargel_check(slot->bitmap.pitch > 0 && slot->bitmap.width == (u32)slot->bitmap.pitch);

        Bitmap bitmap;
        usize width = bitmap.width = slot->bitmap.width;
        usize height = bitmap.height = slot->bitmap.rows;
        bitmap.data.reserve(width * height);
        bitmap.data.set_count(width * height);
        memcpy(bitmap.data.data(), slot->bitmap.buffer, width * height);

        return bitmap;
    }

    GlyphInfo FontLinux::glyphInfo(GlyphId id) {
        if (!ft_face) initFreeType();

        u32 glyph_index = id.value;

        if (FT_Load_Glyph(ft_face, glyph_index, FT_LOAD_NO_SCALE) != 0) {
            spargel_log_fatal("Error calling FT_Load_Glyph (glyph_index=%d).", glyph_index);
            spargel_panic_here();
        }

        auto metrics = ft_face->glyph->metrics;

#define UNIT2POINT(n) ((float)((n) * size) / units_per_EM)
        return GlyphInfo{
            .bounding_box =
                math::Rectangle{.origin = math::Vector2f{.x = UNIT2POINT(metrics.horiBearingX),
                                                         .y = UNIT2POINT(metrics.horiBearingY -
                                                                         metrics.height)},
                                .size = math::RectangleSize{.width = UNIT2POINT(metrics.width),
                                                            .height = UNIT2POINT(metrics.height)}},
            .horizontal_advance = UNIT2POINT(metrics.horiAdvance)};
#undef UNIT2POINT
    }

#if SPARGEL_ENABLE_FREETYPE
    void FontLinux::initFreeType() {
        if (ft_face) return;

        auto error = FT_New_Memory_Face(font_manager->ft_library, buffer.data(), buffer.count(), 0,
                                        &ft_face);
        if (error) {
            if (error == FT_Err_Unknown_File_Format) {
                spargel_log_fatal("Error calling FT_New_Memory_Face: Unsupported file format.");
            } else {
                spargel_log_fatal("Error calling FT_New_Memory_Face.");
            }
            spargel_panic_here();
        }

        units_per_EM = ft_face->units_per_EM;
    }

    void FontLinux::doneFreeType() {
        if (ft_face) {
            FT_Done_Face(ft_face);
            ft_face = nullptr;
        }
    }
#endif

#if SPARGEL_ENABLE_HARFBUZZ
    void FontLinux::initHarfBuzz() {
        if (hb_font) return;

        hb_blob = hb_blob_create(reinterpret_cast<const char*>(buffer.data()), buffer.count(),
                                 HB_MEMORY_MODE_READONLY, nullptr, nullptr);
        if (!hb_blob) {
            spargel_log_fatal("Error calling hb_blob_create.");
            spargel_panic_here();
        }

        hb_face = hb_face_create(hb_blob, 0);
        if (!hb_face) {
            spargel_log_fatal("Error calling hb_face_create.");
            spargel_panic_here();
        }

        hb_font = hb_font_create(hb_face);
        if (!hb_font) {
            spargel_log_fatal("Error calling hb_font_create.");
            spargel_panic_here();
        }
    }

    void FontLinux::doneHarfBuzz() {
        if (hb_font) {
            hb_font_destroy(hb_font);
            hb_face_destroy(hb_face);
            hb_blob_destroy(hb_blob);
            hb_font = nullptr;
        }
    }
#endif

}  // namespace spargel::text
