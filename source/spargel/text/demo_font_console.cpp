#include <spargel/base/logging.h>
#include <spargel/base/string.h>
#include <spargel/math/function.h>
#include <spargel/text/font_freetype.h>
#include <spargel/text/styled_text.h>
#include <spargel/text/text_shaper_harfbuzz.h>

#include <cmath>

// pixels per point
const float XPPP = 5.0f;
const float YPPP = 2.5f;

using namespace spargel;
using namespace spargel::text;

namespace {

    const u32 points_per_inch = 72;

    float max(float a, float b) { return a > b ? a : b; }
    float min(float a, float b) { return a < b ? a : b; }

    base::String getName(const FT_Face& face) {
        char *family_name = face->family_name, *style_name = face->style_name;
        base::String name = base::String(family_name ? family_name : "<unknown>");
        if (style_name) {
            name = name + " (" + base::String(style_name) + ')';
        }
        return name;
    }

    base::UniquePtr<FontFreeType> createFont() {
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

        float size = 10;
        // if (FT_Set_Char_Size(face, (u32)math::round(default_size * 64),
        //                      (u32)math::round(default_size * 64,
        //                      points_per_inch, points_per_inch) != 0) {
        if (FT_Set_Char_Size(face, 0, size * 64, 600, 300) != 0) {
            spargel_log_fatal("Unable to set size for font face %s.",
                              base::CString(getName(face)).data());
            spargel_panic_here();
        }

        return base::makeUnique<FontFreeType>(library, face, size);
    }

    void dump_bitmap(u8* pixels, int pitch, u32 width, u32 height, u8* foreground = nullptr) {
        spargel_check(pitch != 0 && width >= 1 && height >= 1);
        if (pitch > 0) {
            // stored from UP to BOTTOM
            for (usize r = 0; r < height; r++) {
                int last_code = 0;
                for (usize c = 0; c < width; c++) {
                    u8 data = pixels[r * pitch + c];
                    int code = data * (255 - 232) / 256 + 232;
                    if (code != last_code) {
                        printf("\033[38;5;196;48;5;%dm", code);
                        last_code = code;
                    }

                    if (foreground) {
                        putchar(foreground[r * pitch + c]);
                    } else {
                        putchar(' ');
                    }
                }
                printf("\033[0m");
                putchar('\n');
            }
        } else {
            // TODO
            spargel_panic_here();
        }
    }

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        spargel_log_fatal("Usage: %s <text>", argv[0]);
        return 1;
    }

    auto text = base::String(argv[1]);

    spargel_log_info("pixels per point: x=%.3f, y=%.3f", XPPP, YPPP);

    auto font = createFont();
    spargel_log_info("selected font: %s", base::CString{font->name()}.data());

    auto text_shaper = TextShaperHarfBuzz();

    auto shaping_result = text_shaper.shapeLine(StyledText(text.view(), font.get()));
    float x_min = 0, x_max = 0, y_min = 0, y_max = 0;
    float cursor_x = 0, cursor_y = 0;
    for (usize i = 0; i < shaping_result.segments.count(); i++) {
        auto& segment = shaping_result.segments[i];
        spargel_log_info("segment %zu (width=%f) glyph infos:", i, segment.width);
        for (usize j = 0; j < segment.glyphs.count(); j++) {
            auto glyph_id = segment.glyphs[j];

            auto glyph_info = font->glyphInfo(glyph_id);
            auto& pos = segment.positions[j];
            auto& bb_origin = glyph_info.bounding_box.origin;
            auto& bb_size = glyph_info.bounding_box.size;

            spargel_log_info(
                "  [%zu]: id=%d, pos=(%.3f, %.3f), orig=(%.3f, %.3f), size=(%.3f, %.3f), "
                "adv_x=%.3f",
                j, glyph_id.value, pos.x, pos.y, bb_origin.x, bb_origin.y, bb_size.width,
                bb_size.height, glyph_info.horizontal_advance);

            x_min = min(x_min, cursor_x + pos.x + bb_origin.x);
            x_max = max(x_max, cursor_x + pos.x + bb_origin.x + bb_size.width);
            y_min = min(y_min, cursor_y + pos.y + bb_origin.y);
            y_max = max(y_max, cursor_y + pos.y + bb_origin.y + bb_size.height);
        }

        cursor_x += segment.width;
        cursor_y += 0;
    }

    u32 width = (u32)ceil((x_max - x_min) * XPPP) + 4,
        height = (u32)ceil((y_max - y_min) * YPPP) + 4, baseline = (u32)ceil(-y_min * YPPP) + 2;
    spargel_log_info("x_min=%.3f, x_max=%.3f, y_min=%.3f, y_max=%.3f", x_min, x_max, y_min, y_max);
    spargel_log_info("computed bitmap width=%d, height=%d, baseline=%d", width, height, baseline);

    base::vector<u8> pixels;
    pixels.reserve(height * width);
    pixels.set_count(height * width);
    memset(pixels.data(), 0, height * width);

    base::vector<u8> foreground;
    foreground.reserve(height * width);
    foreground.set_count(height * width);
    memset(foreground.data(), ' ', height * width);

    cursor_x = 0;
    cursor_y = 0;
    for (usize i = 0; i < shaping_result.segments.count(); i++) {
        auto& segment = shaping_result.segments[i];
        spargel_log_info("segment %zu final:", i);
        for (usize j = 0; j < segment.glyphs.count(); j++) {
            auto glyph_id = segment.glyphs[j];
            auto pos = segment.positions[j];

            auto glyph_info = font->glyphInfo(glyph_id);
            auto bb_origin = glyph_info.bounding_box.origin;

            auto bitmap = font->rasterGlyphXY(glyph_id, XPPP, YPPP);

            u32 x0 = (u32)math::round((cursor_x + pos.x + bb_origin.x - x_min) * XPPP);
            u32 y0 = (u32)math::round((cursor_y + pos.y + bb_origin.y - y_min) * YPPP);
            spargel_log_info("  [%zu]: id=%d, pos=(%d, %d), bitmap_size=(%zu, %zu)", j,
                             glyph_id.value, x0, y0, bitmap.width, bitmap.height);

            spargel_check(x0 >= 0 && x0 + bitmap.width < width);
            spargel_check(y0 >= 0 && y0 + bitmap.height < height);

            for (usize r = 0; r < bitmap.height; r++) {
                for (usize c = 0; c < bitmap.width; c++) {
                    usize offset = (height - 1 - y0 - bitmap.height + 1 + r) * width + (x0 + c);
                    u8 data = bitmap.data[r * bitmap.width + c];
                    pixels[offset] = max(pixels[offset], data);
                    foreground[offset] = '+';
                }
            }
        }

        cursor_x += segment.width;
        cursor_y += 0;
    }

    // baseline
    memset(foreground.data() + (height - 1 - baseline) * width, '=', width);

    // dump_bitmap(pixels.data(), width, width, height, foreground.data());

    return 0;
}
