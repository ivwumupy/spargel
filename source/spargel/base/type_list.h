#pragma once

#include <spargel/base/types.h>

namespace spargel::base {

    namespace _type_list {

        template <typename... Ts>
        struct TypeList;

        template <typename T>
        struct HeadImpl;

        template <typename T, typename... Ts>
        struct HeadImpl<TypeList<T, Ts...>> {
            using Type = T;
        };

        template <typename T>
        using Head = HeadImpl<T>::Type;

        template <typename T, usize n>
        struct GetImpl;

        template <typename T, typename... Ts>
        struct GetImpl<TypeList<T, Ts...>, 0> {
            using Type = T;
        };

        template <usize n, typename T, typename... Ts>
        struct GetImpl<TypeList<T, Ts...>, n> {
            using Type = GetImpl<TypeList<Ts...>, n - 1>::Type;
        };

        template <typename T, usize n>
        using Get = GetImpl<T, n>::Type;

    }  // namespace _type_list

    using _type_list::Get;
    using _type_list::Head;
    using _type_list::TypeList;

}  // namespace spargel::base
