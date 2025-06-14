#pragma once

#include "spargel/base/concept.h"
#include "spargel/base/types.h"

namespace spargel::base {

    // Allocator 2

    template <typename T>
    concept Allocator2 = requires(T t, usize size) {
        { t.alloc(size) } -> SameAs<void*>;
    } && requires(T t, void* ptr, usize old_size, usize new_size) {
        { t.resize(ptr, old_size, new_size) } -> SameAs<void*>;
    } && requires(T t, void* ptr, usize size) {
        { t.free(ptr, size) } -> SameAs<void>;
    };

}  // namespace spargel::base
