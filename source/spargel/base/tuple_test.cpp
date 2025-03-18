#include <spargel/base/check.h>
#include <spargel/base/test.h>
#include <spargel/base/tuple.h>

using namespace spargel;

TEST(Tuple_Basic) {
    base::Tuple<int, int, bool, double> x;
    x.get<0>() = 1;
    spargel_check(x.get<0>() == 1);
    x.get<1>() = 2;
    spargel_check(x.get<0>() == 1);
    spargel_check(x.get<1>() == 2);
}

TEST(Tuple_Apply) {
    auto f = [](int a, int b) { return a + b; };
    base::Tuple<int, int> x;
    x.get<0>() = 1;
    x.get<1>() = 2;
    auto y = base::apply(f, x);
    spargel_check(y == 3);
}
