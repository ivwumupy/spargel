#pragma once

#include <spargel/math/vector.h>

namespace spargel::math {
    struct RectangleSize {
        float width;
        float height;
    };
    struct Rectangle {
        Vector2f origin;
        RectangleSize size;
    };
}  // namespace spargel::math
