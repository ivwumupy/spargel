#pragma once

#include <spargel/base/object.h>
#include <spargel/base/type_list.h>
#include <spargel/base/types.h>

namespace spargel::base {

    namespace _sum_type {

        template <typename... Ts>
        struct SumTypeStorage;

        template <>
        struct SumTypeStorage<> {};

        template <typename T, typename... Ts>
        struct SumTypeStorage<T, Ts...> {
            SumTypeStorage() {}
            ~SumTypeStorage() {}

            template <usize n>
            Get<TypeList<T, Ts...>, n>& get() {
                if constexpr (n == 0) {
                    return storage.value;
                } else {
                    return storage.next.template get<n - 1>();
                }
            }

            template <usize n>
            void reset() {
                if constexpr (n == 0) {
                    destruct_at(&storage.value);
                } else {
                    storage.next.template reset<n - 1>();
                }
            }

            union _Union {
                _Union() {}
                ~_Union() {}

                T value;
                SumTypeStorage<Ts...> next;
            } storage;
        };

        template <typename... Ts>
        class SumType {
            using Types = TypeList<Ts...>;
            static constexpr usize TypeCount = sizeof...(Ts);

        public:
            SumType() = default;

            ~SumType() = default;

            template <usize n>
            Get<Types, n>& get() {
                return _storage.template get<n>();
            }

            void reset() {
                resetImpl<0>();
                setIndex(-1);
            }

            // unsafe
            void setIndex(usize i) { _index = i; }

            usize index() const { return _index; }

        private:
            template <usize n>
            void resetImpl() {
                if (_index == n) {
                    _storage.template reset<n>();
                } else if constexpr (n + 1 < TypeCount) {
                    resetImpl<n + 1>();
                }
            }

            SumTypeStorage<Ts...> _storage;
            usize _index;
        };

    }  // namespace _sum_type

    using _sum_type::SumType;

}  // namespace spargel::base
