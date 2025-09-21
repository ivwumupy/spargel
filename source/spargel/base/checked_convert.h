#pragma once

#include "spargel/base/check.h"
#include "spargel/base/compiler.h"
#include "spargel/base/limits.h"
#include "spargel/base/meta.h"

namespace spargel::base {
    namespace detail {
        template <typename T>
        inline constexpr bool IsArithmetic =
            // Integrals
            IsSame<T, short> || IsSame<T, unsigned short> || IsSame<T, int> ||
            IsSame<T, unsigned int> || IsSame<T, long> ||
            IsSame<T, unsigned long> || IsSame<T, long long> ||
            IsSame<T, unsigned long long> ||
            // Floating-points
            IsSame<T, float> || IsSame<T, double> || IsSame<T, long double>;

#if spargel_has_builtin(__is_signed)
        template <typename T>
        inline constexpr bool is_signed = __is_signed(T);
#else
        template <typename T>
        inline constexpr bool is_signed = IsArithmetic<T> && T(-1) < T(0);
        static_assert(is_signed<int>);
        static_assert(!is_signed<unsigned int>);
        static_assert(is_signed<float>);
        static_assert(is_signed<double>);
        static_assert(!is_signed<char>);  // Not arithmetic.
#endif
#if spargel_has_builtin(__make_signed) && spargel_has_builtin(__make_unsigned)
        template <typename T>
        using make_signed = __make_signed(T);
        template <typename T>
        using make_unsigned = __make_unsigned(T);
#else
        static_assert(IsSame<int, signed int>);
        template <typename T>
        struct MakeSignedHelper;
        template <typename T>
        struct MakeUnsignedHelper;
#define SPECIALIZE_(T)                      \
    template <>                             \
    struct MakeSignedHelper<signed T> {     \
        using Type = signed T;              \
    };                                      \
    template <>                             \
    struct MakeSignedHelper<unsigned T> {   \
        using Type = signed T;              \
    };                                      \
    template <>                             \
    struct MakeUnsignedHelper<signed T> {   \
        using Type = unsigned T;            \
    };                                      \
    template <>                             \
    struct MakeUnsignedHelper<unsigned T> { \
        using Type = unsigned T;            \
    };
        SPECIALIZE_(short)
        SPECIALIZE_(int)
        SPECIALIZE_(long)
        SPECIALIZE_(long long)
#undef SPECIALIZE_
        template <typename T>
        using make_signed = MakeSignedHelper<T>::Type;
        template <typename T>
        using make_unsigned = MakeUnsignedHelper<T>::Type;
#endif
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
