#pragma once

#include <spargel/base/string_view.h>

namespace spargel::text {
    class Font;

    class StyledText {
    public:
        StyledText(base::StringView s, Font* font) : text_{s}, font_{font} {}

        base::StringView text() const { return text_; }
        Font* font() { return font_; }

    private:
        base::StringView text_;
        Font* font_;
    };
}  // namespace spargel::text
