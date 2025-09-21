#pragma once

#include "spargel/base/concept.h"

namespace spargel::base {
    namespace result_detail {
        template <typename E>
        class Error {
        public:
            // TODO: U is not unexpected or in_place, and can construct E
            template <typename U>
            constexpr explicit Error(U&& u)
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
        Error(E) -> Error<E>;

        template <typename T, typename E>
        class Result {
        public:
            // actually this is value-initialization
            constexpr Result()
                requires(DefaultInitializable<T>)
                : value_{}, has_value_{true} {}

            template <typename... Args>
            constexpr Result(Args&&... args)
                : value_{forward<Args>(args)...}, has_value_{true} {}

            template <typename U>
            constexpr Result(Error<U> const& e)
                : error_{e.error()}, has_value_{false} {}
            template <typename U>
            constexpr Result(Error<U>&& e)
                : error_{move(e).error()}, has_value_{false} {}

            constexpr ~Result() {
                if (has_value_) {
                    destructValue();
                } else {
                    destructError();
                }
            }

            constexpr bool is_error() const { return !has_value_; }

            constexpr Error<E> error() && { return Error{move(error_)}; }

            constexpr T& value() { return value_; }
            constexpr T const& value() const { return value_; }

        private:
            constexpr auto destructValue() -> void {
                if constexpr (!TriviallyDestructible<T>) {
                    value_.~T();
                }
            }
            constexpr auto destructError() -> void {
                if constexpr (!TriviallyDestructible<E>) {
                    error_.~E();
                }
            }

            T value_;
            E error_;
            bool has_value_;
        };
    }  // namespace result_detail
    using result_detail::Error;
    using result_detail::Result;
}  // namespace spargel::base
