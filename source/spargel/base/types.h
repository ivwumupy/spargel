#pragma once

#include <spargel/base/compiler.h>

// c std
#include <stddef.h>
#include <stdint.h>

using ssize = ptrdiff_t;
using usize = size_t;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using nullptr_t = decltype(nullptr);

// TODO
#if !SPARGEL_IS_MSVC
using f16 = _Float16;
#endif
using f32 = float;
using f64 = double;
