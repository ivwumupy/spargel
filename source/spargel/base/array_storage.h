#pragma once

#include <spargel/base/algorithm.h>
#include <spargel/base/allocator.h>
#include <spargel/base/base.h>
#include <spargel/base/meta.h>
#include <spargel/base/tag_invoke.h>
#include <spargel/base/types.h>

namespace spargel::base {

    namespace _array_storage {

        // `ArrayStorage<T>` provides storage for an array of `T`.
        //
        // Note:
        //     It doesn't make sense to copy `ArrayStorage` unless `T` is trivially copyable.
        //     For simplicity, copy is banned unconditionally.
        //
        template <typename T>
        class ArrayStorage {
        public:
            ArrayStorage(Allocator* alloc) : _alloc{alloc} {}

            ArrayStorage(usize count, Allocator* alloc) : _count{count}, _alloc{alloc} {
                _data = static_cast<Byte*>(_alloc->alloc(_count * sizeof(T)));
            }

            ArrayStorage(ArrayStorage const&) = delete;
            ArrayStorage& operator=(ArrayStorage const&) = delete;

            ArrayStorage(ArrayStorage&& other) : _alloc{other._alloc} { swap(*this, other); }
            ArrayStorage& operator=(ArrayStorage&& other) {
                ArrayStorage tmp(move(other));
                swap(*this, tmp);
                return *this;
            }

            ~ArrayStorage() {
                if (_data != nullptr) {
                    _alloc->free(_data, _count * sizeof(T));
                }
            }

            usize count() const { return _count; }

            T* getPtr(usize i) { return reinterpret_cast<T*>(_data + i * sizeof(T)); }
            T const* getPtr(usize i) const {
                return reinterpret_cast<T const*>(_data + i * sizeof(T));
            }

            T& operator[](usize i) { return *getPtr(i); }
            T const& operator[](usize i) const { return *getPtr(i); }

            // T* begin() { return getPtr(0); }
            // T const* begin() const { return getPtr(0); }
            // T* end() { return getPtr(_count - 1); }
            // T const* end() const { return getPtr(_count - 1); }

            friend void tag_invoke(tag<swap>, ArrayStorage& lhs, ArrayStorage& rhs) {
                if (lhs._alloc == rhs._alloc) {
                    swap(lhs._count, rhs._count);
                    swap(lhs._data, rhs._data);
                } else {
                    spargel_panic_here();
                }
            }

        private:
            usize _count = 0;
            // Storage is explicityly provided via a `Byte` array.
            Byte* _data = nullptr;
            Allocator* _alloc = nullptr;
        };

    }  // namespace _array_storage

    using _array_storage::ArrayStorage;

}  // namespace spargel::base
