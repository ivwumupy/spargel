#pragma once

#include "spargel/base/concept.h"

namespace spargel::base {
    namespace _expected {
        template <typename E>
        class Unexpected {
        public:
            // TODO: U is not unexpected or in_place, and can construct E
            template <typename U>
            constexpr explicit Unexpected(U&& u)
                requires(ConstructibleFrom<E, U>)
                : error_{forward<U>(u)} {}

            constexpr auto error() & -> E& { return error_; }
            constexpr auto error() const& -> E const& { return error_; }
            constexpr auto error() && -> E&& { return move(error_); }
            constexpr auto error() const&& -> E const&& { return move(error_); }

        private:
            E error_;
        };
        template <typename E>
        Unexpected(E) -> Unexpected<E>;

        template <typename T, typename E>
        class Expected {
        public:
            // actually this is value-initialization
            constexpr Expected()
                requires(DefaultInitializable<T>)
                : value_{}, has_value_{false} {}

            constexpr ~Expected() {
                if (has_value_) {
                    destruct_value();
                } else {
                    destruct_error();
                }
            }

        private:
            constexpr auto destruct_value() -> void {
                if constexpr (!TriviallyDestructible<T>) {
                    value_.~T();
                }
            }
            constexpr auto destruct_error() -> void {
                if constexpr (!TriviallyDestructible<E>) {
                    error_.~E();
                }
            }

            T value_;
            E error_;
            bool has_value_;
        };
    }  // namespace _expected
    using _expected::Unexpected;
    using _expected::Expected;
}  // namespace spargel::base
