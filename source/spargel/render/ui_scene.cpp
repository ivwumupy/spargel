#include "spargel/render/ui_scene.h"

#include "spargel/base/check.h"
#include "spargel/render/constants.h"
#include "spargel/render/ui_renderer.h"
#include "spargel/text/text_shaper.h"

//
#include <math.h>

namespace spargel::render {
    void UIScene::sampleTexture(float x, float y, float width, float height,
                                UIRenderer::TextureHandle handle, u32 color) {
        auto transform = currentTransform();
        x += transform.x;
        y += transform.y;

        pushCommand(DrawCommand::sample_texture);
        u32 a = (u32)handle.x | (u32)((u32)handle.y << 16);
        u32 b = (u32)handle.width | (u32)((u32)handle.height << 16);
        pushData(x, y, width, height, a, b, color);
        pushCommand2(DrawCommand::sample_texture, x * scale_, y * scale_,
                     width * scale_, height * scale_, a, b, color);

        auto m = (math::ceil(width) + 1.0f) * scale_;
        auto n = (math::ceil(height) + 1.0f) * scale_;
        estimated_slots_ += (usize)((math::ceil(m / 8.0f) + 1.0f) *
                                    (math::ceil(n / 8.0f) + 1.0f));
    }
    void UIScene::fillText(text::StyledText text, float x, float y, u32 color) {
        spargel_check(renderer_);
        auto shaper = renderer_->textShaper();
        auto shape_result = shaper->shapeLine(text);
        for (auto const& segment : shape_result.segments) {
            for (usize i = 0; i < segment.glyphs.count(); i++) {
                auto glyph = segment.glyphs[i];
                auto position = segment.positions[i];

                auto glyph_info = segment.font->glyphInfo(glyph);
                auto const& bbox = glyph_info.bounding_box;

                // The subpixel position of the origin of the bounding box of
                // the glyph. In a bottom-left-origin coordinate.
                math::Vector2f subpixel_position;
                math::Vector2f integral_position;
                subpixel_position.x =
                    modff((x + position.x + bbox.origin.x) * scale_,
                          &integral_position.x);
                subpixel_position.y =
                    1.0f - modff((y - position.y - bbox.origin.y) * scale_,
                                 &integral_position.y);

                UIRenderer::TextureHandle handle;

                // TODO: This is broken.
                //
                if constexpr (ENABLE_SUBPIXEL_QUANTIZATION) {
                    math::Vector2f subpixel_variantf;
                    subpixel_position.x =
                        modff(subpixel_position.x * SUBPIXEL_SUBDIVISION,
                              &subpixel_variantf.x) /
                        SUBPIXEL_SUBDIVISION;
                    subpixel_position.y =
                        modff(subpixel_position.y * SUBPIXEL_SUBDIVISION,
                              &subpixel_variantf.y) /
                        SUBPIXEL_SUBDIVISION;

                    handle = renderer_->prepareGlyph(
                        glyph, segment.font,
                        SubpixelVariant{static_cast<u8>(subpixel_variantf.x),
                                        static_cast<u8>(subpixel_variantf.y)});
                } else {
                    handle = renderer_->prepareGlyph(glyph, segment.font,
                                                     subpixel_position);
                }

                // NOTE: It's expected that the bitmap of the rasterized glyph
                // has enough space for anti-aliasing as follows. Assume that
                // the bitmap uses the glyph coordinate space, i.e. the origin
                // is at bottom left. Then the origin of the glyph should be
                // placed at `1 + subpixel_shift`. This accounts for the +1/-1
                // on the `integral_position` below.
                if (handle.width > 0 && handle.height > 0) {
                    sampleTexture(
                        (integral_position.x - 1) / scale_,
                        (integral_position.y + 2 - handle.height) / scale_,
                        handle.width / scale_, handle.height / scale_, handle,
                        color);
                }
            }
        }
    }
}  // namespace spargel::render
