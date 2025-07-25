#pragma once

#include "spargel/base/check.h"
#include "spargel/base/compiler.h"
#include "spargel/base/panic.h"
#include "spargel/base/types.h"

namespace spargel::base {

    inline u8 GetMostSignificantBit(u64 x) {
        spargel_check(x > 0);

#if spargel_has_builtin(__builtin_clzll)
        // __builtin_clz:
        //   Returns the number of leading 0-bits in x, starting at the most
        //   significant bit position. If x is 0, the result is undefined.
        return static_cast<u8>(63 - __builtin_clzll(x));
#else
        spargel_panic_here();
#endif
    }

}  // namespace spargel::base
