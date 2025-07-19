#include <spargel/render/atlas_packer.h>

namespace spargel::render {
    namespace {
        u16 max(u16 a, u16 b) { return a > b ? a : b; }
        inline constexpr u16 GAP = 2;
    }  // namespace
    base::Optional<PackResult> AtlasPacker::pack(u16 width, u16 height) {
        if (current_column_ + GAP + width >= width_) {
            current_row_ = next_row_;
            current_column_ = 0;
        }
        u16 x = current_column_ + GAP;
        current_column_ += GAP + width;
        next_row_ = max(next_row_, current_row_ + height);
        if (next_row_ + GAP >= height_) {
            return base::nullopt;
        }
        u16 y = current_row_ + GAP;
        return base::makeOptional<PackResult>(x, y);
    }
}  // namespace spargel::render
