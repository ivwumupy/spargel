#include "spargel/base/check.h"
#include "spargel/base/checked_convert.h"
#include "spargel/base/test.h"

namespace spargel::base {
    namespace {
        template <typename T>
        [[clang::noinline]]
        decltype(auto) blackbox(T&& v) {
            return forward<T>(v);
        }

        TEST(CheckedConvert_Basic) {
            spargel_check(checkedConvert<u8>(int(blackbox(0))) == u8(0));
            spargel_check(checkedConvert<u8>(int(1)) == u8(1));
            spargel_check(checkedConvert<u8>(int(127)) == u8(127));
        }
    }  // namespace
}  // namespace spargel::base
