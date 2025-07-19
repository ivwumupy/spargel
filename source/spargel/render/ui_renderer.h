#pragma once

#include <spargel/gpu/gpu_context.h>
#include <spargel/math/vector.h>
#include <spargel/render/atlas_packer.h>
#include <spargel/text/font.h>

namespace spargel::text {
    class TextShaper;
}

namespace spargel::render {
    class UIScene;

    class UIRenderer {
    public:
        // TODO: Redesign.
        struct TextureHandle {
            u16 x;
            u16 y;
            u16 width;
            u16 height;
        };

        static constexpr usize ATLAS_SIZE = 2048;

        virtual ~UIRenderer() = default;

        gpu::GPUContext* context() { return context_; }
        text::TextShaper* text_shaper() { return text_shaper_; }

        virtual void render(UIScene const& scene) = 0;

        TextureHandle prepareGlyph(text::GlyphId id, text::Font* font);
        TextureHandle prepareGlyph(text::GlyphId id, text::Font* font, math::Vector2f subpixel_position);

        void setScaleFactor(float s) { scale_factor_ = s; }
        float scale_factor() const { return scale_factor_; }

    protected:
        explicit UIRenderer(gpu::GPUContext* context, text::TextShaper* text_shaper)
            : context_{context}, text_shaper_{text_shaper}, packer_{ATLAS_SIZE, ATLAS_SIZE} {}

        virtual void uploadBitmap(TextureHandle handle, text::Bitmap const& bitmap) = 0;

    private:
        struct GlyphItem {
            text::GlyphId id;
            text::Font* font;
        };

        gpu::GPUContext* context_;
        text::TextShaper* text_shaper_;
        AtlasPacker packer_;
        float scale_factor_ = 1.0;
    };
    base::UniquePtr<UIRenderer> makeUIRenderer(gpu::GPUContext* context,
                                               text::TextShaper* text_shaper);
}  // namespace spargel::render
