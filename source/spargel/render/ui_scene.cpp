#include <spargel/base/check.h>
#include <spargel/base/logging.h>
#include <spargel/render/ui_renderer.h>
#include <spargel/render/ui_scene.h>
#include <spargel/text/text_shaper.h>

namespace spargel::render {
    void UIScene::sampleTexture(float x, float y, float width, float height,
                                UIRenderer::TextureHandle handle, u32 color) {
        pushCommand(DrawCommand::sample_texture);
        u32 a = (u32)handle.x | (u32)((u32)handle.y << 16);
        u32 b = (u32)handle.width | (u32)((u32)handle.height << 16);
        spargel_log_info("sample %u %u %u %u", handle.x, handle.y, handle.width, handle.height);
        pushData(x, y, width, height, a, b, color);
    }
    void UIScene::fillText(text::StyledText text, float x, float y, u32 color) {
        spargel_check(renderer_);
        auto shaper = renderer_->text_shaper();
        auto shape_result = shaper->shapeLine(text);
        for (auto const& segment : shape_result.segments) {
            for (usize i = 0; i < segment.glyphs.count(); i++) {
                auto glyph = segment.glyphs[i];
                auto position = segment.positions[i];

                auto handle = renderer_->prepareGlyph(glyph, segment.font);
                auto glyph_info = segment.font->glyphInfo(glyph);

                auto const& bbox = glyph_info.bounding_box;

                spargel_log_info("glyph_info: %.3f %.3f", glyph_info.width(), glyph_info.height());
                spargel_log_info("position: %.3f %.3f", position.x, position.y);
                spargel_log_info("bbox: %.3f %.3f", bbox.origin.x, bbox.origin.y);

                if (handle.width > 0 && handle.height > 0) {
                    sampleTexture(x + position.x + bbox.origin.x,
                                  y - position.y - bbox.origin.y - bbox.size.height,
                                  bbox.size.width, bbox.size.height, handle, color);
                }
            }
        }
    }
}  // namespace spargel::render
