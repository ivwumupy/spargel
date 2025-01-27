#pragma once

#include <spargel/base/types.h>

namespace spargel::base {

    [[noreturn]] void panic();

    [[noreturn]] void panic_at(char const* file, char const* func, u32 line);

#define spargel_panic_here() ::spargel::base::panic_at(__FILE__, __func__, __LINE__)

}  // namespace spargel::base
