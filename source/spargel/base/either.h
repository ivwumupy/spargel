#pragma once

#include <spargel/base/meta.h>
#include <spargel/base/sum_type.h>

namespace spargel::base {

    namespace _either {

        template <typename L, typename R>
        class Either {
            using Type = SumType<L, R>;

        public:
            // Constructors
            Either() = default;
            Either(const Either&) = default;

            template <typename L2, typename R2>
                requires(is_convertible<L2, L> && is_convertible<R2, R>)
            Either(Either<L2, R2>&& other)
                : _value(other.isLeft() ? Type::template make<0>(move(other.left()))
                                        : Type::template make<1>(move(other.right()))) {}

            // Assignment operators
            Either& operator=(const Either&) = default;
            Either& operator=(Either&&) = default;

            bool isLeft() const { return _value.getIndex() == 0; }
            bool isRight() const { return _value.getIndex() == 1; }

            // Value accessors
            L& left() { return _value.template getValue<0>(); }
            const L& left() const { return _value.template getValue<0>(); }

            R& right() { return _value.template getValue<1>(); }
            const R& right() const { return _value.template getValue<1>(); }

            template <typename G, typename D, typename... Arg>
            friend Either<G, D> makeLeft(Arg&&... args);

            template <typename G, typename D, typename... Arg>
            friend Either<G, D> makeRight(Arg&&... args);

        private:
            template <bool>
            struct LeftTag {};

            template <typename... Arg>
            Either(LeftTag<true>, Arg&&... args)
                : _value(Type::template make<0>(forward<Arg>(args)...)) {}

            template <typename... Arg>
            Either(LeftTag<false>, Arg&&... args)
                : _value(Type::template make<1>(forward<Arg>(args)...)) {}

            Type _value;
        };

        template <typename L, typename R, typename... Arg>
        Either<L, R> makeLeft(Arg&&... args) {
            return Either<L, R>(typename Either<L, R>::template LeftTag<true>{},
                                forward<Arg>(args)...);
        }

        template <typename L, typename R, typename... Arg>
        Either<L, R> makeRight(Arg&&... args) {
            return Either<L, R>(typename Either<L, R>::template LeftTag<false>{},
                                forward<Arg>(args)...);
        }

    }  // namespace _either

    using _either::Either;
    using _either::makeLeft;
    using _either::makeRight;

}  // namespace spargel::base
