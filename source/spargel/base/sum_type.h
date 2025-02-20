#pragma once

#include <spargel/base/algorithm.h>
#include <spargel/base/meta.h>
#include <spargel/base/object.h>
#include <spargel/base/tag_invoke.h>
#include <spargel/base/type_list.h>
#include <spargel/base/types.h>

namespace spargel::base {

    namespace _sum_type {

        template <typename... Ts>
        struct StorageSizeImpl;

        template <typename T>
        struct StorageSizeImpl<T> {
            static constexpr usize value = sizeof(T);
        };

        template <typename T, typename... Ts>
        struct StorageSizeImpl<T, Ts...> {
            static constexpr usize _rest = StorageSizeImpl<Ts...>::value;
            static constexpr usize value = sizeof(T) > _rest ? sizeof(T) : _rest;
        };

        template <typename... Ts>
        inline constexpr usize StorageSize = StorageSizeImpl<Ts...>::value;

        /// SumpType cannot be empty.
        template <typename... Ts>
        class SumType {
            using Types = TypeList<Ts...>;
            static constexpr usize TypeCount = sizeof...(Ts);

            template <usize n>
            struct IndexWrapper {};

        public:
            template <usize i, typename... Args>
            static SumType make(Args&&... args) {
                return SumType(IndexWrapper<i>{}, forward<Args>(args)...);
            }

            ~SumType() {}

            usize getIndex() const { return _index; }
            void setIndex(usize i) { _index = i; }

            template <usize i, typename T = Get<Types, i>>
            T& getValue() { return *getPtr<T>(); }

            friend void tag_invoke(tag<swap>, SumType& lhs, SumType& rhs) {
                if (lhs._index == rhs._index) {

                } else {

                }
            }

        private:
            template <usize i, typename... Args>
            SumType(IndexWrapper<i>, Args&&... args) : _index{i} {
                construct_at(getPtr<Get<Types, i>>(), forward<Args>(args)...);
            }

            template <typename T>
            T* getPtr() { return reinterpret_cast<T*>(_bytes); }

            // TODO: Alignment.
            Byte _bytes[StorageSize<Ts...>];

            usize _index;
        };

    }  // namespace _sum_type

    using _sum_type::SumType;

}  // namespace spargel::base
