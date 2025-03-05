#pragma once

#include <spargel/base/meta.h>
#include <spargel/base/types.h>

namespace spargel::base {

    namespace __curry {

        template <usize N, typename F, typename... BoundArgs>
        struct Curry;

        template <typename F>
        struct Curry<0, F> {
            F f;

            constexpr Curry(F&& f) : f(f) {}

            template <typename... CallArgs>
            constexpr auto operator()(CallArgs&&... args) {
                return f(forward<CallArgs>(args)...);
            }
        };

        template <typename F, typename T>
        struct Curry<1, F, T> {
            F f;
            T t;

            constexpr Curry(F&& f, T&& t) : f(f), t(t) {}

            template <typename... CallArgs>
            constexpr auto operator()(CallArgs&&... args) {
                return f(t, forward<CallArgs>(args)...);
            }
        };

        template <usize N, typename F, typename T, typename... BoundArgs>
        struct Curry<N, F, T, BoundArgs...> {
            Curry<N - 1, F, BoundArgs...> g;
            T t;

            constexpr Curry(F&& f, BoundArgs&&... args, T&& t)
                : g(Curry<N - 1, F, BoundArgs...>(forward<F>(f), forward<BoundArgs>(args)...)),
                  t(t) {}

            template <typename... CallArgs>
            constexpr auto operator()(CallArgs&&... args) {
                return g(t, forward<CallArgs>(args)...);
            }
        };

        template <typename F, typename... Args>
        Curry(F&&, Args&&...) -> Curry<sizeof...(Args), F, Args...>;

    }  // namespace __curry

    using __curry::Curry;

}  // namespace spargel::base
