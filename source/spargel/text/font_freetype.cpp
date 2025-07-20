#include <spargel/text/font_freetype.h>

const u32 XPPI = 600;
const u32 YPPI = 300;

using namespace spargel::base::literals;

namespace spargel::text {

    FreeTypeLibrary defaultFreeTypeLibrary;

    FontFreeType::FontFreeType(FT_Library library, FT_Face face) : library(library), face(face) {
        char* family_name = face->family_name;
        name_ = base::String(family_name ? family_name : "<unknown>");
    }

    FontFreeType::~FontFreeType() { FT_Done_Face(face); }

    Bitmap FontFreeType::rasterGlyph(GlyphId id, float scale) {
        loadGlyph(id.value);

        auto& slot = face->glyph;
        if (FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL) != 0) {
            spargel_log_fatal("Unable to render glyph %d from font face %s.", id.value,
                              base::CString(name_).data());
            spargel_panic_here();
        }

        Bitmap bitmap;
        u32 width = slot->bitmap.width, height = slot->bitmap.rows;
        u32 count = width * height;
        bitmap.width = width;
        bitmap.height = height;
        bitmap.data.reserve(count);
        bitmap.data.set_count(count);
        memcpy(bitmap.data.data(), slot->bitmap.buffer, count);

        return bitmap;
    }

    GlyphInfo FontFreeType::glyphInfo(GlyphId id) {
        loadGlyph(id.value);
        auto& metrics = face->glyph->metrics;

        GlyphInfo info;
        info.bounding_box =
            math::Rectangle{.origin = {metrics.horiBearingX / 64.0f,
                                       (metrics.horiBearingY - metrics.height) / 64.0f},
                            .size = {metrics.width / 64.0f, metrics.height / 64.0f}};
        info.horizontal_advance = metrics.horiAdvance / 64.0f;
        return info;
    }

    void FontFreeType::loadGlyph(u32 id) {
        if (FT_Load_Glyph(face, id, FT_LOAD_NO_BITMAP) != 0) {
            spargel_log_fatal("Unable to load glyph %d from font face %s.", id,
                              base::CString(name_).data());
            spargel_panic_here();
        }
    }

    base::UniquePtr<Font> createDefaultFont() {
        auto library = defaultFreeTypeLibrary.library;
        auto filename = "/usr/share/fonts/truetype/msttcorefonts/Times_New_Roman.ttf";
        // auto filename = "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc";
        // auto filename = "/usr/share/fonts/truetype/noto/NotoSansMongolian-Regular.ttf";
        // filename = "/home/jerry/Data/fonts/PerfectoCalligraphy.ttf";

        FT_Face face;
        auto error = FT_New_Face(library, filename, 0, &face);
        if (error == FT_Err_Unknown_File_Format) {
            spargel_log_fatal("Unsupported font format");
            spargel_panic_here();
        } else if (error) {
            spargel_log_fatal("Unable to create font face from buffer");
            spargel_panic_here();
        }

        if (FT_Set_Char_Size(face, 0, 10 * 64, XPPI, YPPI) != 0) {
            spargel_log_fatal(
                "Unable to set size for font face %s.",
                face->family_name ? base::CString(face->family_name).data() : "<unknown>");
            spargel_panic_here();
        }

        return base::makeUnique<FontFreeType>(library, face);
    }

}  // namespace spargel::text
