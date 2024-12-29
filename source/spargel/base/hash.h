#pragma once

#include <spargel/base/compiler.h>
#include <spargel/base/types.h>

// todo: now the code assumes little-endian
// use `__builtin_bswap64` (gcc/clang) or `_byteswap_uint64` (msvc) to swap endian

namespace spargel::base {

    namespace __wyhash {

        // read three bytes
        inline constexpr u64 wyread3(void const* data, u64 len) {
            auto p = static_cast<u8 const*>(data);
            u64 a = p[0];
            u64 b = p[len >> 1];
            u64 c = p[len - 1];
            return (a << 56) | (b << 32) | c;
        }

        inline constexpr u64 wyread4(void const* data) {
            u32 v;
#if spargel_has_builtin(__builtin_memcpy)
            __builtin_memcpy(&v, data, 4);
#else
#error unimplemented
#endif
            return v;
        }

        inline constexpr u64 wyread8(void const* data) {
            u64 v;
#if spargel_has_builtin(__builtin_memcpy)
            __builtin_memcpy(&v, data, 8);
#else
#error unimplemented
#endif
            return v;
        }

        inline constexpr bool protect_mode = false;

        inline constexpr void wymul(u64& a, u64& b) {
#ifdef __SIZEOF_INT128__
            __uint128_t r = a;
            r *= b;
            if constexpr (protect_mode) {
                a ^= (u64)r;
                b ^= (u64)(r >> 64);
            } else {
                a = (u64)r;
                b = (u64)(r >> 64);
            }
#else
#error unimplemented
#endif
        }

        inline constexpr u64 wymix(u64 a, u64 b) {
            wymul(a, b);
            return a ^ b;
        }

        inline constexpr u64 secret0 = 0x2d358dccaa6c78a5;
        inline constexpr u64 secret1 = 0x8bb84b93962eacc9;
        inline constexpr u64 secret2 = 0x4b33a62ed433d4a3;

        inline constexpr u64 default_seed = 0xbdd89aa982704029;

        inline constexpr u64 wyhash(void const* data, u64 len, u64 seed) {
            u64 a;
            u64 b;

            auto p = static_cast<u8 const*>(data);

            if (len <= 16) [[likely]] {
                if (len >= 4) {
                    auto plast = p + len - 4;
                    // how about ((len&24)>>(len>>3))
                    auto delta = (len >> 3) << 2;
                    a = (wyread4(p) << 32) | wyread4(plast);
                    b = (wyread4(p + delta) << 32) | wyread4(plast - delta);
                } else if (len > 0) {
                    a = wyread3(data, len);
                    b = 0;
                } else {
                    a = 0;
                    b = 0;
                }
            } else {
                u64 i = len;
                if (i > 48) [[unlikely]] {
                    u64 seed1 = seed;
                    u64 seed2 = seed;
                    do {
                        seed = wymix(wyread8(p) ^ secret0, wyread8(p + 8) ^ seed);
                        seed1 = wymix(wyread8(p + 16) ^ secret1, wyread8(p + 24) ^ seed1);
                        seed2 = wymix(wyread8(p + 32) ^ secret2, wyread8(p + 40) ^ seed2);
                        p += 48;
                        i -= 48;
                    } while (i >= 48);
                    seed ^= seed1 ^ seed2;
                }
                if (i > 16) {
                    seed = wymix(wyread8(p) ^ secret2, wyread8(p + 8) ^ seed ^ secret1);
                    if (i > 32) {
                        seed = wymix(wyread8(p + 16) ^ secret2, wyread8(p + 24) ^ seed);
                    }
                }
                a = wyread8(p + i - 16);
                b = wyread8(p + i - 8);
            }
            a ^= secret1;
            b ^= seed;
            wymul(a, b);
            return wymix(a ^ secret0 ^ len, b ^ secret1);
        }

    }  // namespace __wyhash

}  // namespace spargel::base
