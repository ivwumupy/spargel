#include <spargel/base/check.h>
#include <spargel/base/logging.h>
#include <spargel/render/ui_renderer.h>
#include <spargel/render/ui_scene.h>
#include <spargel/text/text_shaper.h>

//
#include <math.h>

namespace spargel::render {
    void UIScene::sampleTexture(float x, float y, float width, float height,
                                UIRenderer::TextureHandle handle, u32 color) {
        pushCommand(DrawCommand::sample_texture);
        u32 a = (u32)handle.x | (u32)((u32)handle.y << 16);
        u32 b = (u32)handle.width | (u32)((u32)handle.height << 16);
        // spargel_log_info("sample: x=%u y=%u w=%u h=%u", handle.x, handle.y, handle.width,
        //                  handle.height);
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

                auto glyph_info = segment.font->glyphInfo(glyph);
                auto const& bbox = glyph_info.bounding_box;

                // The subpixel position of the origin of the bounding box of the glyph.
                // In a bottom-left-origin coordinate.
                math::Vector2f subpixel_position;
                math::Vector2f integral_position;
                subpixel_position.x = modff(x + position.x + bbox.origin.x, &integral_position.x);
                subpixel_position.y = 1.0f - modff(y - position.y - bbox.origin.y, &integral_position.y);

                auto handle = renderer_->prepareGlyph(glyph, segment.font, subpixel_position);

                // Before scaled.
                // spargel_log_info("glyph_info: w=%.3f h=%.3f", glyph_info.width(),
                //                  glyph_info.height());
                // spargel_log_info("position: %.3f %.3f", position.x, position.y);
                // spargel_log_info("bbox: x=%.3f y=%.3f", bbox.origin.x, bbox.origin.y);

                float scale = renderer_->scale_factor();

                spargel_log_info("%.3f %.3f",
                        integral_position.y + 1.0f - handle.height / scale,
                        floorf(y - position.y - bbox.origin.y - bbox.size.height));

                if (handle.width > 0 && handle.height > 0) {
                    //sampleTexture(x + position.x + bbox.origin.x,
                    //              y - position.y - bbox.origin.y - bbox.size.height,
                    //              bbox.size.width, bbox.size.height, handle, color);
                    sampleTexture(integral_position.x,
                                  integral_position.y + 1.0f - handle.height / scale,
                                  handle.width / scale, handle.height / scale, handle, color);
                }
            }
        }
    }
}  // namespace spargel::render
