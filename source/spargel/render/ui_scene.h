#pragma once

#include <spargel/base/bit_cast.h>
#include <spargel/base/enum.h>
#include <spargel/base/string_view.h>
#include <spargel/base/types.h>
#include <spargel/base/vector.h>

namespace spargel::render {
    // Coordinates: The origin is at top-left.
    class UIScene {
    public:
        void fillRect(float x, float y, float w, float h, u32 color) {
            pushCommand(DrawCommand::fill_rect);
            pushData(x, y, w, h, color);
        }
        void fillCircle(float x, float y, float r, u32 color) {
            pushCommand(DrawCommand::fill_circle);
            pushData(x, y, r, color);
        }
        void strokeLine(float x0, float y0, float x1, float y1, u32 color) {
            pushCommand(DrawCommand::stroke_line);
            pushData(x0, y0, x1, y1, color);
        }
        void strokeCircle(float x, float y, float r, u32 color) {
            pushCommand(DrawCommand::stroke_circle);
            pushData(x, y, r, color);
        }
        // This does not support emojis.
        //
        // TODO: Fonts.
        void fillText(base::StringView s, float x, float y, u32 color);

        base::Span<u8> commands() const { return commands_.toSpan(); }
        base::Span<u32> data() const { return data_.toSpan(); }

    private:
        enum class DrawCommand : u8 {
            fill_rect = 0,
            fill_circle,
            stroke_line,
            stroke_circle,
        };

        void pushCommand(DrawCommand cmd) { commands_.push(base::toUnderlying(cmd)); }
        void pushDatum(u32 x) { data_.push(x); }
        void pushDatum(float x) { data_.push(base::bitCast<float, u32>(x)); }
        template <typename... Ts>
        void pushData(Ts... ts) { (pushDatum(ts), ...); }

        base::Vector<u8> commands_;
        base::Vector<u32> data_;
    };
}  // namespace spargel::render
