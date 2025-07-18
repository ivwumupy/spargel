#include <spargel/text/font_freetype.h>
#include <spargel/text/styled_text.h>
#include <spargel/text/text_shaper_harfbuzz.h>

// HarfBuzz-FreeType
#include <harfbuzz/hb-ft.h>

namespace spargel::text {

    ShapedLine TextShaperHarfBuzz::shapeLine(const StyledText& text) {
        // FIXME
        auto font = static_cast<FontFreeType*>(text.font());
        hb_font_t* font_hb = hb_ft_font_create(font->face, nullptr);

        // TODO: text runs
        ShapedSegment segment;
        segment.font = text.font();

        hb_buffer_t* buf = hb_buffer_create();

        auto text_view = text.text();
        hb_buffer_add_utf8(buf, text_view.data(), text_view.length(), 0, text_view.length());

        hb_buffer_set_direction(buf, HB_DIRECTION_LTR);
        hb_buffer_set_script(buf, HB_SCRIPT_LATIN);
        hb_buffer_set_language(buf, hb_language_from_string("en", -1));

        hb_shape(font_hb, buf, nullptr, 0);

        unsigned int glyph_count;
        hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buf, &glyph_count);
        hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(buf, &glyph_count);

        segment.glyphs.reserve(glyph_count);
        segment.glyphs.set_count(glyph_count);
        segment.positions.reserve(glyph_count);
        segment.positions.set_count(glyph_count);

        hb_position_t cursor_x = 0;
        hb_position_t cursor_y = 0;
        for (unsigned int i = 0; i < glyph_count; i++) {
            hb_codepoint_t glyph_id = glyph_info[i].codepoint;
            hb_position_t x_offset = glyph_pos[i].x_offset;
            hb_position_t y_offset = glyph_pos[i].y_offset;
            hb_position_t x_advance = glyph_pos[i].x_advance;
            hb_position_t y_advance = glyph_pos[i].y_advance;

            segment.glyphs[i] = GlyphId{glyph_id};
            segment.positions[i].x = cursor_x + x_offset;
            segment.positions[i].y = cursor_y + y_offset;

            cursor_x += x_advance;
            cursor_y += y_advance;
        }

        ShapedLine result;
        result.segments.emplace(base::move(segment));

        hb_buffer_destroy(buf);

        return result;
    }

    base::UniquePtr<TextShaper> TextShaper::create() {
        return base::makeUnique<TextShaperHarfBuzz>();
    }

}  // namespace spargel::text
