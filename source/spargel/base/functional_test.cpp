#include <spargel/base/check.h>
#include <spargel/base/functional.h>
#include <spargel/base/test.h>

// libc
#include <stdio.h>

using namespace spargel::base;

constexpr int f(int a, int b, int c, int d) { return a + b + c + d; }

static_assert(Curry(f)(1, 2, 3, 4) == 10);
static_assert(Curry(f, 1)(2, 3, 4) == 10);
static_assert(Curry(f, 1, 2)(3, 4) == 10);
static_assert(Curry(f, 1, 2, 3)(4) == 10);

TEST(Curry) {
    spargel_check(Curry(f)(1, 2, 3, 4) == 10);
    spargel_check(Curry(f, 1)(2, 3, 4) == 10);
    spargel_check(Curry(f, 1, 2)(3, 4) == 10);
    spargel_check(Curry(f, 1, 2, 3)(4) == 10);

    Curry(printf, "%d %s %c\n")(1, "ABC", '@');

    int a = 1;
    auto f = [&a](int b, int c, int d) { return a + b + c + d; };
    spargel_check(Curry(f)(2, 3, 4) == 10);
    spargel_check(Curry(f, 2)(3, 4) == 10);
    spargel_check(Curry(f, 2, 3)(4) == 10);
}
