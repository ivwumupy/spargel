#pragma once

#include "spargel/math/vector.h"

namespace spargel::math {
    struct RectangleSize {
        float width;
        float height;
    };
    struct Rectangle {
        Vector2f origin;
        RectangleSize size;

        bool contains(Vector2f p) const {
            return (p.x >= origin.x) && (p.y >= origin.y) && (p.x <= origin.x + size.width) &&
                   (p.y <= origin.y + size.height);
        }
    };
}  // namespace spargel::math
