#include <spargel/base/logging.h>
#include <spargel/base/string.h>
#include <spargel/text/font.h>
#include <spargel/text/styled_text.h>
#include <spargel/text/text_shaper.h>

//
#include <stdio.h>

const u32 XPPI = 600;
const u32 YPPI = 300;

using namespace spargel;

template <typename T>
T max(T a, T b) {
    return a > b ? a : b;
}

namespace {

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

    spargel_log_info("xppi=%d, yppi=%d", XPPI, YPPI);

    auto font = text::createDefaultFont();
    spargel_log_info("Selected font: %s", base::CString{font->name()}.data());

    auto text_shaper = text::TextShaper::create();

    auto shaping_result = text_shaper->shapeLine(text::StyledText(text.view(), font.get()));
    float total_width = 0, max_ascent = 0, max_descent = 0, left_more = 0, right_more = 0;
    for (usize i = 0; i < shaping_result.segments.count(); i++) {
        auto& segment = shaping_result.segments[i];
        printf("Segment %zu (width=%f):\n", i, segment.width);
        for (usize j = 0; j < segment.glyphs.count(); j++) {
            auto glyph_id = segment.glyphs[j];

            auto glyph_info = font->glyphInfo(glyph_id);

            auto pos = segment.positions[j];
            auto bb_origin = glyph_info.bounding_box.origin;
            auto bb_size = glyph_info.bounding_box.size;

            printf(
                "  %zu: glyph_id=%d, position=(%f, %f), origin=(%f, %f), size=(%f, %f), "
                "advance_x=%f\n",
                j, glyph_id.value, pos.x, pos.y, bb_origin.x, bb_origin.y, bb_size.width,
                bb_size.height, glyph_info.horizontal_advance);

            max_ascent = max(max_ascent, glyph_info.ascent());
            max_descent = max(max_descent, -glyph_info.descent());

            left_more = max(left_more, -(pos.x + bb_origin.x));
            right_more = max(right_more, pos.x + bb_origin.x + bb_size.width - segment.width);
        }
        total_width += segment.width;
    }
    total_width = total_width + left_more + right_more + 2;
    max_ascent += 1;
    max_descent += 1;

    u32 height = (u32)(max_ascent + max_descent), baseline = (u32)max_descent,
        width = (u32)total_width;
    spargel_log_debug("final w=%d, h=%d, baseline=%d", width, height, baseline);
    base::vector<u8> pixels;
    pixels.reserve(height * width);
    pixels.set_count(height * width);
    memset(pixels.data(), 0, height * width);

    base::vector<u8> foreground;
    foreground.reserve(height * width);
    foreground.set_count(height * width);
    memset(foreground.data(), ' ', height * width);

    u32 segment_cursor_x = left_more + 1;
    for (auto& segment : shaping_result.segments) {
        for (usize i = 0; i < segment.glyphs.count(); i++) {
            auto glyph_id = segment.glyphs[i];
            auto pos = segment.positions[i];
            u32 x = (u32)pos.x, y = (u32)pos.y;

            auto glyph_info = font->glyphInfo(glyph_id);
            auto bb_origin = glyph_info.bounding_box.origin;

            auto bitmap = font->rasterGlyph(glyph_id, 1.0f);

            u32 x0 = segment_cursor_x + x + (u32)bb_origin.x;
            u32 y0 = y + (u32)bb_origin.y;

            spargel_check(x0 >= 0 && x0 + bitmap.width < width);
            spargel_check(baseline + y0 >= 0 && baseline + y0 + bitmap.height < height);

            for (usize r = 0; r < bitmap.height; r++) {
                for (usize c = 0; c < bitmap.width; c++) {
                    usize offset =
                        (height - 1 - baseline - y0 - bitmap.height + 1 + r) * width + (x0 + c);
                    u8 data = bitmap.data[r * bitmap.width + c];
                    pixels[offset] = max(pixels[offset], data);
                    foreground[offset] = '+';
                }
            }
        }

        segment_cursor_x += width;
    }

    // baseline
    memset(foreground.data() + (height - 1 - baseline) * width, '=', width);

    dump_bitmap(pixels.data(), width, width, height, foreground.data());

    return 0;
}
