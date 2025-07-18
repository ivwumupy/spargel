#pragma once

#include <spargel/base/bit_cast.h>
#include <spargel/base/hash.h>
#include <spargel/base/string_view.h>

namespace spargel::text {
    class Font;

    class StyledText {
    public:
        StyledText(base::StringView s, Font* font) : text_{s}, font_{font} {}

        base::StringView text() const { return text_; }
        Font* font() { return font_; }

        friend bool operator==(StyledText const& lhs, StyledText const& rhs) {
            return lhs.text_ == rhs.text_ && lhs.font_ == rhs.font_;
        }

        friend void tag_invoke(base::tag<base::hash>, base::HashRun& run, StyledText const& self) {
            run.combine(self.text_);
            run.combine(base::bitCast<Font*, u64>(self.font_));
        }

    private:
        base::StringView text_;
        Font* font_;
    };
}  // namespace spargel::text
