#pragma once

#include "spargel/base/bit_cast.h"
#include "spargel/base/enum.h"
#include "spargel/base/string_view.h"
#include "spargel/base/types.h"
#include "spargel/base/vector.h"
#include "spargel/math/rectangle.h"
#include "spargel/render/ui_renderer.h"
#include "spargel/text/styled_text.h"

namespace spargel::text {
    class Font;
}

namespace spargel::render {
    class UIRenderer;

    // Coordinates: The origin is at top-left.
    //
    // V1: Commands and data are separated.
    //
    // V2: Combined command with a uniform size.
    //
    class UIScene {
    public:
        void setRenderer(UIRenderer* renderer) { renderer_ = renderer; }

        // TODO: Clip stack.
        void setClip(float x, float y, float w, float h) {
            pushCommand(DrawCommand::set_clip);
            pushData(x, y, w, h);
            clip_ = math::Rectangle{{x * scale_, y * scale_}, {w * scale_, h * scale_}};
        }
        void clearClip() {
            pushCommand(DrawCommand::clear_clip);
            clip_ = math::Rectangle{};
        }

        void fillRect(float x, float y, float w, float h, u32 color) {
            pushCommand(DrawCommand::fill_rect);
            pushData(x, y, w, h, color);
            pushCommand2(DrawCommand::fill_rect, x * scale_, y * scale_, w * scale_, h * scale_, color);
        }
        void fillCircle(float x, float y, float r, u32 color) {
            pushCommand(DrawCommand::fill_circle);
            pushData(x, y, r, color);
            pushCommand2(DrawCommand::fill_circle, x * scale_, y * scale_, r * scale_, color);
        }
        void strokeLine(float x0, float y0, float x1, float y1, u32 color) {
            pushCommand(DrawCommand::stroke_line);
            pushData(x0, y0, x1, y1, color);
            pushCommand2(DrawCommand::stroke_line, x0 * scale_, y0 * scale_, x1 * scale_, y1 * scale_, color);
        }
        void strokeCircle(float x, float y, float r, u32 color) {
            pushCommand(DrawCommand::stroke_circle);
            pushData(x, y, r, color);
            pushCommand2(DrawCommand::stroke_circle, x * scale_, y * scale_, r * scale_, color);
        }
        // Render a line of text with the given font and color.
        //
        // TODO: Support emojis.
        void fillText(text::StyledText text, float x, float y, u32 color);

        void strokeRectangle(float x, float y, float w, float h, u32 color) {
            strokeLine(x, y, x + w, y, color);
            strokeLine(x + w, y, x + w, y + h, color);
            strokeLine(x + w, y + h, x, y + h, color);
            strokeLine(x, y + h, x, y, color);
        }

        void dump() { pushCommand(DrawCommand::dump); }

        base::Span<u8> commands() const { return commands_.toSpan(); }
        base::Span<u32> data() const { return data_.toSpan(); }

        base::Span<base::Byte> commands2_bytes() const { return commands2_.toSpan().asBytes(); }

        void clear() {
            commands_.clear();
            data_.clear();
        }

        void setScale(float s) { scale_ = s; }

    private:
        enum class DrawCommand : u8 {
            fill_rect = 0,
            fill_circle,
            stroke_line,
            stroke_circle,
            set_clip,
            clear_clip,
            sample_texture,
            dump,
        };

        // TODO: 15 bytes are wasted.
        struct Command2 {
            DrawCommand cmd;
            // This is float4.
            alignas(16) math::Rectangle clip;
            u32 data[8];
        };
        static_assert(sizeof(Command2) == 64);

        void pushCommand(DrawCommand cmd) { commands_.push(base::toUnderlying(cmd)); }
        void pushDatum(u32 x) { data_.push(x); }
        void pushDatum(float x) { data_.push(base::bitCast<float, u32>(x * scale_)); }
        template <typename... Ts>
        void pushData(Ts... ts) {
            (pushDatum(ts), ...);
        }

        void sampleTexture(float x, float y, float width, float height,
                           UIRenderer::TextureHandle handle, u32 color);

        template <typename... Ts>
        void pushCommand2(DrawCommand cmd, Ts... ts) {
            commands2_.push(
                Command2{cmd, clip_,
                    {base::bitCast<Ts, u32>(ts)...}});
        }

        float scale_ = 1.0;
        UIRenderer* renderer_ = nullptr;
        base::Vector<u8> commands_;
        base::Vector<u32> data_;

        math::Rectangle clip_;
        base::Vector<Command2> commands2_;
    };
}  // namespace spargel::render
