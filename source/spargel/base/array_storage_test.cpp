#include <spargel/base/allocator.h>
#include <spargel/base/array_storage.h>
#include <spargel/base/check.h>
#include <spargel/base/meta.h>
#include <spargel/base/test.h>

using namespace spargel::base;

TEST(ArrayStorage_DefaultConstructor) {
    ArrayStorage<int> y;
    spargel_check(y.getCount() == 0);
}

TEST(ArrayStorage_ConstructByCapacity) {
    ArrayStorage<int> x{10};
    spargel_check(x.getCount() == 10);
}

TEST(ArrayStorage_MoveConstructor) {
    ArrayStorage<int> x{10};
    ArrayStorage<int> z(move(x));
    spargel_check(x.getCount() == 0);  // NOLINT(clang-analyzer-cplusplus.Move)
    spargel_check(z.getCount() == 10);
}
