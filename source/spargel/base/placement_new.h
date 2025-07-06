#pragma once

inline constexpr void* operator new(__SIZE_TYPE__ count, void* ptr) noexcept { return ptr; }
inline constexpr void* operator new[](__SIZE_TYPE__ count, void* ptr) noexcept { return ptr; }
