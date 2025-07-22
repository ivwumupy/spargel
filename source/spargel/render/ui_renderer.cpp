#include <spargel/base/checked_convert.h>
#include <spargel/base/logging.h>
#include <spargel/config.h>
#include <spargel/render/ui_renderer.h>
#include <stdio.h>

namespace spargel::render {

#if SPARGEL_IS_MACOS
    base::UniquePtr<UIRenderer> makeMetalUIRenderer(gpu::GPUContext* context,
                                                    text::TextShaper* shaper);
#endif

    base::UniquePtr<UIRenderer> makeUIRenderer(gpu::GPUContext* context, text::TextShaper* shaper) {
#if SPARGEL_IS_MACOS
        return makeMetalUIRenderer(context, shaper);
#endif
        return nullptr;
    }
    // TODO: Cache.
    UIRenderer::TextureHandle UIRenderer::prepareGlyph(text::GlyphId id, text::Font* font) {
        auto bitmap = font->rasterGlyph(id, scale_factor_);
        auto width = base::checkedConvert<u16>(bitmap.width);
        auto height = base::checkedConvert<u16>(bitmap.height);
        auto pack_result = packer_.pack(width, height);
        if (!pack_result.hasValue()) {
            spargel_log_error("cannot pack glyph");
            spargel_panic_here();
        }
        TextureHandle handle{pack_result.value().x, pack_result.value().y, width, height};
        if (width > 0 && height > 0) {
            uploadBitmap(handle, bitmap);
        }
        return handle;
    }
    UIRenderer::TextureHandle UIRenderer::prepareGlyph(text::GlyphId id, text::Font* font,
                                                       SubpixelVariant subpixel) {
        GlyphCacheKey key{id, font, subpixel};
        auto* result = glyph_cache_.get(key);
        if (result) {
            return *result;
        }
        spargel_log_info("glyph cache miss: %u %p %d %d", id.value, (void*)font, subpixel.x, subpixel.y);
        // TODO: Do not hardcode 0.25.
        auto handle = prepareGlyph(id, font, math::Vector2f{(float)subpixel.x * 0.25f, (float)subpixel.y * 0.25f});
        glyph_cache_.set(key, handle);
        spargel_check(glyph_cache_.get(key));
        return handle;
    }
    UIRenderer::TextureHandle UIRenderer::prepareGlyph(text::GlyphId id, text::Font* font,
                                                       math::Vector2f subpixel_position) {
        auto bitmap = font->rasterGlyph(id, scale_factor_, subpixel_position);
        auto width = base::checkedConvert<u16>(bitmap.width);
        auto height = base::checkedConvert<u16>(bitmap.height);
        auto pack_result = packer_.pack(width, height);
        if (!pack_result.hasValue()) {
            spargel_log_error("cannot pack glyph");
            spargel_panic_here();
        }
        TextureHandle handle{pack_result.value().x, pack_result.value().y, width, height};
        if (width > 0 && height > 0) {
            uploadBitmap(handle, bitmap);
        }
        return handle;
    }
}  // namespace spargel::render
