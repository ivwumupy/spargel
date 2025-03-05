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

            template <typename G, typename D, typename... Args>
            friend Either<G, D> makeLeft(Args&&... args);

            template <typename G, typename D, typename... Args>
            friend Either<G, D> makeRight(Args&&... args);

        private:
            template <bool>
            struct LeftTag {};

            template <typename... Args>
            Either(LeftTag<true>, Args&&... args)
                : _value(Type::template make<0>(forward<Args>(args)...)) {}

            template <typename... Args>
            Either(LeftTag<false>, Args&&... args)
                : _value(Type::template make<1>(forward<Args>(args)...)) {}

            Type _value;
        };

        template <typename L, typename R, typename... Args>
        Either<L, R> makeLeft(Args&&... args) {
            return Either<L, R>(typename Either<L, R>::template LeftTag<true>{},
                                forward<Args>(args)...);
        }

        template <typename L, typename R, typename... Args>
        Either<L, R> makeRight(Args&&... args) {
            return Either<L, R>(typename Either<L, R>::template LeftTag<false>{},
                                forward<Args>(args)...);
        }

    }  // namespace _either

    using _either::Either;
    using _either::makeLeft;
    using _either::makeRight;

}  // namespace spargel::base
