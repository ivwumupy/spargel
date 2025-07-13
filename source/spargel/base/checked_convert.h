#pragma once

#include "spargel/base/limits.h"
#include "spargel/base/check.h"

namespace spargel::base {
    namespace detail {
        template <typename T>
        inline constexpr bool is_signed = __is_signed(T);
        template <typename T>
        using make_signed = __make_signed(T);
        template <typename T>
        using make_unsigned = __make_unsigned(T);
        template <typename T, typename U>
        constexpr bool safe_less_equal(T t, U u) {
            if constexpr (is_signed<T> == is_signed<U>) {
                return t <= u;
            } else if constexpr (is_signed<T>) {
                return t < 0 ? true : make_unsigned<T>(t) <= u;
            } else {
                return u < 0 ? false : t <= make_unsigned<U>(u);
            }
        }
    }  // namespace detail
    // for numeric types only
    template <typename To, typename From>
    constexpr To checkedConvert(From v) {
        spargel_check(detail::safe_less_equal(v, NumericLimits<To>::max));
        spargel_check(detail::safe_less_equal(NumericLimits<To>::min, v));
        return static_cast<To>(v);
    }

}  // namespace spargel::base
