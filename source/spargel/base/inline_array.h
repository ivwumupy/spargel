#pragma once

#include <spargel/base/assert.h>
#include <spargel/base/types.h>

namespace spargel::base {

    template <typename T, usize size>
    struct InlineArray {
        T& operator[](usize i) {
            spargel_assert(i < size);
            return _array[i];
        }
        T const& operator[](usize i) const {
            spargel_assert(i < size);
            return _array[i];
        }

        usize count() const { return size; }

        T _array[size];
    };

}  // namespace spargel::base
