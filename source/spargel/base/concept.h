#pragma once

#include <spargel/base/meta.h>

namespace spargel::base {

    // note:
    // - std::is_convertible, i.e. __is_convertible, checks whether From is
    //   implicitly convertible to To.
    // - std::convertible_to checks whether From is both implicitly and explicitly
    //   (via static_cast) to To.
    // - for one example of the difference, see [stackoverflow/62644070]
    template <typename From, typename To>
    concept ConvertibleTo = base::is_convertible<From, To> && requires { static_cast<To>(declval<From>()); };

    template <typename S, typename T>
    concept SameAs = base::IsSame<S, T>;

    template <typename T>
    concept Destructible = __is_nothrow_destructible(T);
    template <typename T, typename... Args>
    concept ConstructibleFrom = Destructible<T> && __is_constructible(T, Args...);
    // note:
    // - default-initialization
    //   - T t;
    //   - new T;
    // - value-initialization: an object is constructed with an empty initializer
    //   - T();     // not T t(); this is ambiguous
    //   - new T();
    //   - T t{};
    //   - T{};
    //   - new T{};
    //   - constructor() : member() {}
    //   - constructor() : member{} {}
    template <typename T>
    concept DefaultInitializable = ConstructibleFrom<T> && requires {
        T{};
        ::new T;
    };
    template <typename T>
    concept TriviallyDestructible = __is_trivially_destructible(T);

}  // namespace spargel::base
