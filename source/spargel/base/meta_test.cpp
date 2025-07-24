#include "spargel/base/meta.h"

#include "spargel/base/test.h"

using namespace spargel::base;

static_assert(IsSame<bool, bool>);
static_assert(IsSame<int, int>);
static_assert(!IsSame<int, bool>);

static_assert(IsSame<conditional<true, int, bool>, int>);
static_assert(IsSame<conditional<false, int, bool>, bool>);

static_assert(is_array<int[]>);
static_assert(is_array<int[1]>);
static_assert(is_array<int[1][2]>);
static_assert(!is_array<int>);
static_assert(!is_array<int*>);
static_assert(!is_array<int&>);

static_assert(IsSame<remove_extent<int[]>, int>);
static_assert(IsSame<remove_extent<int[1]>, int>);
static_assert(IsSame<remove_extent<int[1][2]>, int[2]>);

static_assert(IsSame<add_pointer<int>, int*>);
static_assert(IsSame<add_pointer<int&>, int*>);

static_assert(!is_function<int>);
static_assert(!is_function<int&>);
static_assert(is_function<int(int)>);

static_assert(IsSame<decay<int>, int>);
static_assert(IsSame<decay<int&>, int>);
static_assert(IsSame<decay<int&&>, int>);
static_assert(IsSame<decay<int const&>, int>);
static_assert(IsSame<decay<int const&&>, int>);
static_assert(IsSame<decay<int[1]>, int*>);
static_assert(IsSame<decay<int[1][2]>, int (*)[2]>);
static_assert(IsSame<decay<int(int)>, int (*)(int)>);

static_assert(!IsSame<decay<int>, bool>);

TEST(Meta_Stub) {}
