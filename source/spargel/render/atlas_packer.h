#pragma once

#include <spargel/base/optional.h>
#include <spargel/base/types.h>

namespace spargel::render {
    struct PackResult {
        u16 x;
        u16 y;
    };
    // An online rect packer.
    //
    // Currently this is a dummy implementation.
    //
    // TODO: Support deletion and reusing space.
    //
    class AtlasPacker {
    public:
        AtlasPacker(u16 width, u16 height)
            : width_{width}, height_{height}, current_row_{}, current_column_{}, next_row_{} {}

        u16 width() const { return width_; }
        u16 height() const { return height_; }

        base::Optional<PackResult> pack(u16 width, u16 height);

    private:
        u16 width_;
        u16 height_;

        u16 current_row_;
        u16 current_column_;
        u16 next_row_;
    };
}  // namespace spargel::render
