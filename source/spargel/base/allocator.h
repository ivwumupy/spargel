#pragma once

#include "spargel/base/meta.h"
#include "spargel/base/object.h"
#include "spargel/base/placement_new.h"  // IWYU pragma: keep
#include "spargel/base/types.h"

namespace spargel::base {

    class Allocator {
    public:
        virtual ~Allocator() = default;

        // requires: size > 0
        virtual void* allocate(usize size) = 0;
        // requires: ptr != nullptr, old_size > 0, new_size > 0, old_size < new_size
        virtual void* resize(void* ptr, usize old_size, usize new_size) = 0;
        // requires: ptr != nullptr, size > 0
        virtual void free(void* ptr, usize size) = 0;

        template <typename T, typename... Args>
        T* allocObject(Args&&... args) {
            T* ptr = static_cast<T*>(allocate(sizeof(T)));
            construct_at<T>(ptr, forward<Args>(args)...);
            return ptr;
        }

        template <typename T>
        void freeObject(T* ptr) {
            ptr->~T();
            free(ptr, sizeof(T));
        }
    };

    Allocator* default_allocator();

}  // namespace spargel::base
