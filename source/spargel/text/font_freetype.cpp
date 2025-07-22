#include <spargel/math/function.h>
#include <spargel/text/font_freetype.h>

using namespace spargel::base::literals;

namespace spargel::text {

    namespace {

        const u32 points_per_inch = 72;

        base::String getName(const FT_Face& face) {
            char *family_name = face->family_name, *style_name = face->style_name;
            base::String name = base::String(family_name ? family_name : "<unknown>");
            if (style_name) {
                name = name + " (" + base::String(style_name) + ')';
            }
            return name;
        }

    }  // namespace

    FreeTypeLibrary defaultFreeTypeLibrary;

    FontFreeType::FontFreeType(FT_Library library, FT_Face face, float size)
        : library(library), face(face), size(size), name_(getName(face)) {}

    FontFreeType::~FontFreeType() { FT_Done_Face(face); }

    Bitmap FontFreeType::rasterGlyphXY(GlyphId id, float scale_x, float scale_y) {
        if (FT_Set_Char_Size(face, (u32)math::round(size * 64), (u32)math::round(size * 64),
                             (u32)math::round(scale_x * points_per_inch),
                             (u32)math::round(scale_y * points_per_inch)) != 0) {
            spargel_log_fatal("rasterGlyphXY: unable to set size for font \"%s\".",
                              base::CString(getName(face)).data());
            spargel_panic_here();
        }

        loadGlyph(id.value);

        auto& slot = face->glyph;
        if (FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL) != 0) {
            spargel_log_fatal("rasterGlyphXY: unable to render glyph %d from font \"%s\".",
                              id.value, base::CString(name_).data());
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

    Bitmap FontFreeType::rasterGlyph(GlyphId id, float scale) {
        return rasterGlyphXY(id, scale, scale);
    }

    GlyphInfo FontFreeType::glyphInfo(GlyphId id) {
        // reset to default
        /*if (FT_Set_Char_Size(face, (u32)math::round(size * 64), (u32)math::round(size * 64),  //
                             points_per_inch, points_per_inch) != 0) {
            spargel_log_fatal("rasterGlyphXY: unable to set size for font \"%s\".",
                              base::CString(getName(face)).data());
            spargel_panic_here();
        }*/

        loadGlyph(id.value);
        auto& metrics = face->glyph->metrics;
        spargel_log_debug("x=%ld, y=%ld, w=%ld, h=%ld, a=%ld", metrics.horiBearingX,
                          metrics.horiBearingY, metrics.width, metrics.height, metrics.horiAdvance);

        GlyphInfo info;
        info.bounding_box = math::Rectangle{
            .origin = {.x = metrics.horiBearingX / 64.0f,
                       .y = (metrics.horiBearingY - metrics.height) / 64.0f},
            .size = {.width = metrics.width / 64.0f, .height = metrics.height / 64.0f}};
        info.horizontal_advance = metrics.horiAdvance / 64.0f;
        return info;
    }

    void FontFreeType::loadGlyph(u32 id) {
        if (FT_Load_Glyph(face, id, FT_LOAD_DEFAULT) != 0) {
            spargel_log_fatal("Unable to load glyph %d from font %s.", id,
                              base::CString(name_).data());
            spargel_panic_here();
        }
    }

    base::UniquePtr<Font> createDefaultFont() {
        auto library = defaultFreeTypeLibrary.library;
        auto filename = "/usr/share/fonts/truetype/msttcorefonts/Times_New_Roman.ttf";

        FT_Face face;
        auto error = FT_New_Face(library, filename, 0, &face);
        if (error == FT_Err_Unknown_File_Format) {
            spargel_log_fatal("Unsupported font format.");
            spargel_panic_here();
        } else if (error) {
            spargel_log_fatal("Unable to create font face from buffer.");
            spargel_panic_here();
        }

        float default_size = 12;
        if (FT_Set_Char_Size(face, (u32)math::round(default_size * 64),
                             (u32)math::round(default_size * 64), points_per_inch,
                             points_per_inch) != 0) {
            spargel_log_fatal("Unable to set size for font %s.",
                              base::CString(getName(face)).data());
            spargel_panic_here();
        }

        return base::makeUnique<FontFreeType>(library, face, default_size);
    }

}  // namespace spargel::text
