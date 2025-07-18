#include <spargel/resource/directory.h>
#include <spargel/text/font_freetype.h>

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

        FT_GlyphSlot slot = face->glyph;
        if (FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL) != 0) {
            spargel_log_fatal("Unable to render glyph %d from font face %s.", id.value,
                              base::CString(name_).data());
            spargel_panic_here();
        }

        // FIXME: upside-down
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

        // FIXME: upside-down
        GlyphInfo info;
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
        auto font_filename = "/usr/share/fonts/truetype/msttcorefonts/Times_New_Roman.ttf"_sv;

        auto resource_manager = resource::ResourceManagerDirectory(""_sv);
        auto font_file = resource_manager.open(resource::ResourceId(font_filename));
        if (!font_file.hasValue()) {
            spargel_log_fatal("Cannot open font resource \"%s\"",
                              base::CString(font_filename).data());
            spargel_panic_here();
        }
        auto buffer = font_file.value()->getSpan();

        FT_Face face;
        auto error = FT_New_Memory_Face(library, buffer.data(), buffer.count(), 0, &face);
        if (error == FT_Err_Unknown_File_Format) {
            spargel_log_fatal("Unsupported font format");
            spargel_panic_here();
        } else if (error) {
            spargel_log_fatal("Unable to create font face from buffer");
            spargel_panic_here();
        }

        if (FT_Set_Pixel_Sizes(face, 0, 16) != 0) {
            spargel_log_fatal(
                "Unable to set pixel size for font face %s.",
                face->family_name ? base::CString(face->family_name).data() : "<unknown>");
            spargel_panic_here();
        }

        return base::makeUnique<FontFreeType>(library, face);
    }

}  // namespace spargel::text
