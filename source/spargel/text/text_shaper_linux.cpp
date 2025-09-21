#include "spargel/text/text_shaper_linux.h"

#include "spargel/text/font_linux.h"
#include "spargel/text/styled_text.h"

namespace spargel::text {

#if SPARGEL_ENABLE_HARFBUZZ
    ShapedLine TextShaperLinux::shapeLine(const StyledText& text) {
        auto font = static_cast<FontLinux*>(text.font());
        if (!font->hb_font) font->initHarfBuzz();

        // TODO: text runs
        ShapedSegment segment;
        segment.font = text.font();

        hb_buffer_t* buf = hb_buffer_create();

        auto text_view = text.text();
        hb_buffer_add_utf8(buf, text_view.data(), text_view.length(), 0,
                           text_view.length());

        hb_buffer_set_direction(buf, HB_DIRECTION_LTR);
        hb_buffer_set_script(buf, HB_SCRIPT_LATIN);
        hb_buffer_set_language(buf, hb_language_from_string("en", -1));

        hb_shape(font->hb_font, buf, nullptr, 0);

        u32 glyph_count;
        hb_glyph_info_t* glyph_info =
            hb_buffer_get_glyph_infos(buf, &glyph_count);
        hb_glyph_position_t* glyph_pos =
            hb_buffer_get_glyph_positions(buf, &glyph_count);

        segment.glyphs.reserve(glyph_count);
        segment.glyphs.set_count(glyph_count);
        segment.positions.reserve(glyph_count);
        segment.positions.set_count(glyph_count);

        float cursor_x = 0;
        float cursor_y = 0;
        for (u32 i = 0; i < glyph_count; i++) {
            hb_codepoint_t glyph_id = glyph_info[i].codepoint;

            segment.glyphs[i] = GlyphId{glyph_id};
            segment.positions[i].x = cursor_x + glyph_pos[i].x_offset / 64.0f;
            segment.positions[i].y = cursor_y + glyph_pos[i].y_offset / 64.0f;

            cursor_x += glyph_pos[i].x_advance / 64.0f;
            cursor_y += glyph_pos[i].y_advance / 64.0f;
        }
        segment.width = cursor_x;

        ShapedLine result;
        result.segments.emplace(base::move(segment));

        hb_buffer_destroy(buf);

        return result;
    }
#else
#error "unimplemented"
#endif

    base::UniquePtr<TextShaper> TextShaper::create(FontManager* font_manager) {
        return base::makeUnique<TextShaperLinux>();
    }

}  // namespace spargel::text
