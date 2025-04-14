#pragma once

#include <spargel/base/meta.h>
#include <spargel/base/type_list.h>

namespace spargel::base {

    namespace _tuple {

        template <usize... Is>
        struct IntegerSequence {};

        template <typename T, usize I>
        struct Append;
        template <usize I, usize... Js>
        struct Append<IntegerSequence<Js...>, I> {
            using Type = IntegerSequence<Js..., I>;
        };
        template <usize N>
        struct MakeSequence {
            using Type = Append<typename MakeSequence<N - 1>::Type, N - 1>::Type;
        };
        template <>
        struct MakeSequence<0> {
            using Type = IntegerSequence<>;
        };

        // L is a TypeList
        template <usize i, typename L>
        struct TupleItem {
            using Type = Get<L, i>;
            Type data;
        };

        // L is a TypeList
        template <typename L, typename I>
        struct TupleStorage;
        template <typename L, usize... Is>
        struct TupleStorage<L, IntegerSequence<Is...>> : TupleItem<Is, L>... {
            template <usize i>
            auto& get() {
                return TupleItem<i, L>::data;
            }
        };

        template <typename... Ts>
        class Tuple {
        public:
            // TODO: constructors

            template <usize i>
            auto& get() {
                return _storage.template get<i>();
            }

        private:
            TupleStorage<TypeList<Ts...>, typename MakeSequence<sizeof...(Ts)>::Type> _storage;
        };

        template <typename T>
        struct TupleSize;

        template <typename... Ts>
        struct TupleSize<Tuple<Ts...>> {
            static constexpr usize value = sizeof...(Ts);
        };

        template <typename F, typename T, usize... Is>
        decltype(auto) applyImpl(F&& func, T&& tuple, IntegerSequence<Is...>) {
            return base::forward<F>(func)(base::forward<T>(tuple).template get<Is>()...);
        }

        template <typename F, typename T>
        decltype(auto) apply(F&& func, T&& tuple) {
            return applyImpl(base::forward<F>(func), base::forward<T>(tuple),
                             typename MakeSequence<TupleSize<base::decay<T>>::value>::Type{});
        }

    }  // namespace _tuple

    using _tuple::Tuple;

    using _tuple::apply;

}  // namespace spargel::base
