#pragma once

// note: math.h of libstdc++ includes a definition of placement new
#include <new>  // IWYU pragma: keep

// inline constexpr void* operator new(__SIZE_TYPE__ count, void* ptr) noexcept
// { return ptr; } inline constexpr void* operator new[](__SIZE_TYPE__ count,
// void* ptr) noexcept { return ptr; }
