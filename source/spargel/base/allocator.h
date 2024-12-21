#pragma once

#include <spargel/base/types.h>

// the default placement new cannot be overrided
#include <new>

namespace spargel::base {

    struct Allocator {
        // requires: size > 0
        virtual void* alloc(usize size) = 0;
        // requires: ptr != nullptr, old_size > 0, new_size > 0
        virtual void* resize(void* ptr, usize old_size, usize new_size) = 0;
        // requires: ptr != nullptr, size > 0
        virtual void free(void* ptr, usize size) = 0;
    };

    struct libc_allocator final : Allocator {
        static libc_allocator* instance();

        void* alloc(usize size) override;
        void* resize(void* ptr, usize old_size, usize new_size) override;
        void free(void* ptr, usize size) override;
    };

    Allocator* default_allocator();

}  // namespace spargel::base

void* operator new(usize size, usize align) noexcept;
void* operator new[](usize size, usize align) noexcept;
void operator delete(void* ptr, usize size, usize align) noexcept;
void operator delete[](void* ptr, usize size, usize align) noexcept;

void* operator new(usize size, usize align, spargel::base::Allocator* alloc) noexcept;
