#include "spargel/base/string_view.h"

#include "spargel/base/check.h"
#include "spargel/base/test.h"

TEST(StringView_Literal) {
    using namespace spargel::base::literals;
    auto x = "hello, world"_sv;
    spargel_check(x.length() == 12);
}
