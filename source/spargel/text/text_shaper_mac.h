#pragma once

#include "spargel/text/text_shaper.h"

namespace spargel::text {
    class FontManagerMac;

    class TextShaperMac final : public TextShaper {
    public:
        TextShaperMac(FontManagerMac* manager) : font_manager_{manager} {}
        ShapedLine shapeLine(StyledText const& text) override;
    private:
        FontManagerMac* font_manager_;
    };
}  // namespace spargel::text
