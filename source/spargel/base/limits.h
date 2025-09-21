#pragma once

#include "spargel/base/types.h"

namespace spargel::base {

    template <typename T>
    struct NumericLimits {
        static constexpr bool is_signed = T(-1) < T(0);
        static constexpr usize digits =
            static_cast<int>(sizeof(T) * 8 - is_signed);
        static constexpr T min = is_signed ? T(T(1) << digits) : T(0);
        static constexpr T max = is_signed ? T(T(~0) ^ min) : T(~0);
    };
}  // namespace spargel::base
