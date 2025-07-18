#pragma once

#include <spargel/base/span.h>
#include <spargel/base/vector.h>

namespace spargel::text {
    class StyledText;

    class ShapedSegment {
    public:
    private:
    };
    class ShapedLine {
    public:
        base::Span<ShapedSegment> segments() const { return segments_.toSpan(); }

    private:
        base::Vector<ShapedSegment> segments_;
    };

    class TextShaper {
    public:
        virtual ~TextShaper() = default;

        virtual ShapedLine shapeLine(StyledText const& text) = 0;
    };
}  // namespace spargel::text
