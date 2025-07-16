#pragma once

namespace spargel::base {
    // The underlying type of an enum.
    //
    // TODO: Check that T is an enum.
    template <typename T>
    using UnderlyingType = __underlying_type(T);

    template <typename T>
    [[nodiscard]] constexpr UnderlyingType<T> toUnderlying(T t) {
        return static_cast<UnderlyingType<T>>(t);
    }

    // NOTE: It's not possible to implement a generic safe fromUnderlying without reflection.
}  // namespace spargel::base
