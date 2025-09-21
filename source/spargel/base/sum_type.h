#pragma once

#include "spargel/base/algorithm.h"
#include "spargel/base/check.h"
#include "spargel/base/meta.h"
#include "spargel/base/object.h"
#include "spargel/base/tag_invoke.h"
#include "spargel/base/type_list.h"
#include "spargel/base/types.h"

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
            static constexpr usize value = sizeof(T) > _rest ? sizeof(T)
                                                             : _rest;
        };

        template <typename... Ts>
        inline constexpr usize StorageSize = StorageSizeImpl<Ts...>::value;

        template <typename... Ts>
        struct AlignmentImpl;

        template <typename T>
        struct AlignmentImpl<T> {
            static constexpr usize value = alignof(T);
        };

        template <typename T, typename... Ts>
        struct AlignmentImpl<T, Ts...> {
            static constexpr usize _rest = AlignmentImpl<Ts...>::value;
            static constexpr usize value = alignof(T) > _rest ? alignof(T)
                                                              : _rest;
        };

        template <typename... Ts>
        inline constexpr usize Alignment = AlignmentImpl<Ts...>::value;

        template <usize n>
        struct IndexWrapper {};

        template <usize N, usize i = 0, typename F>
        decltype(auto) visitByIndex(usize id, F&& f) {
            if constexpr (i >= N) {
                return;
            } else if (i == id) {
                return f(IndexWrapper<i>{});
            } else {
                return visitByIndex<N, i + 1>(id, base::forward<F>(f));
            }
        }

        /// SumpType cannot be empty.
        ///
        /// Note: When swapping two `SumType`, a temporary object will be
        /// created.
        ///
        template <typename... Ts>
        class SumType {
            using Types = TypeList<Ts...>;
            static constexpr usize TypeCount = sizeof...(Ts);

        public:
            template <usize i, typename... Args>
            static SumType make(Args&&... args) {
                return SumType(IndexWrapper<i>{}, forward<Args>(args)...);
            }

            SumType(SumType const& other) : index_{other.index_} {
                visitByIndex<TypeCount>(index_, [&]<usize i>(IndexWrapper<i>) {
                    using T = Get<Types, i>;
                    new (getPtr<T>()) T(other.getValue<i>());
                });
            }
            SumType& operator=(SumType const& other) {
                SumType tmp(other);
                swap(*this, tmp);
                return *this;
            }

            SumType(SumType&& other) : index_{other.index_} {
                visitByIndex<TypeCount>(index_, [&]<usize i>(IndexWrapper<i>) {
                    using T = Get<Types, i>;
                    new (getPtr<T>()) T(base::move(other.getValue<i>()));
                });
            }
            SumType& operator=(SumType&& other) {
                SumType tmp(base::move(other));
                swap(*this, tmp);
                return *this;
            }

            ~SumType() {
                visitByIndex<TypeCount>(index_, [&]<usize i>(IndexWrapper<i>) {
                    using T = Get<Types, i>;
                    destruct_at(getPtr<T>());
                });
            }

            usize index() const { return index_; }
            usize getIndex() const { return index_; }
            // unsafe
            void setIndex(usize i) {
                spargel_check(i < TypeCount);
                index_ = i;
            }

            template <usize i, typename T = Get<Types, i>>
            T& value() {
                return *getPtr<T>();
            }
            template <usize i, typename T = Get<Types, i>>
            T const& value() const {
                return *getPtr<T>();
            }
            template <usize i, typename T = Get<Types, i>>
            T& getValue() {
                return *getPtr<T>();
            }
            template <usize i, typename T = Get<Types, i>>
            T const& getValue() const {
                return *getPtr<T>();
            }

            friend void tag_invoke(tag<swap>, SumType& lhs, SumType& rhs) {
                if (lhs.index_ == rhs.index_) {
                    visitByIndex<TypeCount>(
                        lhs.index_, [&]<usize i>(IndexWrapper<i>) {
                            swap(lhs.getValue<i>(), rhs.getValue<i>());
                        });
                } else {
                    alignas(Alignment<Ts...>) Byte tmp[StorageSize<Ts...>];
                    visitByIndex<TypeCount>(lhs.index_, [&]<usize i>(
                                                            IndexWrapper<i>) {
                        // MSVC requires `SumType::`.
                        visitByIndex<SumType::TypeCount>(
                            rhs.index_, [&]<usize j>(IndexWrapper<j>) {
                                using T1 = Get<Types, i>;
                                using T2 = Get<Types, j>;
                                new (reinterpret_cast<T1*>(tmp))
                                    T1(base::move(lhs.getValue<i>()));
                                new (lhs.getPtr<T2>())
                                    T2(base::move(rhs.getValue<j>()));
                                new (rhs.getPtr<T1>())
                                    T1(base::move(*reinterpret_cast<T1*>(tmp)));
                            });
                    });
                    swap(lhs.index_, rhs.index_);
                }
            }

        private:
            template <usize i, typename... Args>
            SumType(IndexWrapper<i>, Args&&... args) : index_{i} {
                construct_at(getPtr<Get<Types, i>>(), forward<Args>(args)...);
            }

            template <typename T>
            T* getPtr() {
                return reinterpret_cast<T*>(bytes_);
            }
            template <typename T>
            T const* getPtr() const {
                return reinterpret_cast<T const*>(bytes_);
            }

            usize index_;
            alignas(Alignment<Ts...>) Byte bytes_[StorageSize<Ts...>];
        };

    }  // namespace _sum_type

    using _sum_type::SumType;

    template <typename... Ts>
    using Variant = SumType<Ts...>;

}  // namespace spargel::base
