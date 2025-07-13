#pragma once

#include <stdlib.h>

#include "spargel/base/types.h"

namespace spargel::base {

    // Allocator 2
    //
    // struct Alloc {
    //   auto alloc(usize size) -> void*;
    //   auto resize(void* ptr, usize new_size) -> void*;
    //   auto free(void* ptr) -> void;
    // };

    struct DefaultAllocator {
        auto alloc(usize size) -> void* {
            return ::malloc(size);
        }
    };

}  // namespace spargel::base
