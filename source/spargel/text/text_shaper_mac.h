#pragma once

#include <spargel/text/text_shaper.h>

namespace spargel::text {
    class TextShaperMac final : public TextShaper {
    public:
        ShapedLine shapeLine(StyledText const& text) override;
    };
}  // namespace spargel::text
