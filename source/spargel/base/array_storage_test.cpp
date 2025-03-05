#include <spargel/base/allocator.h>
#include <spargel/base/array_storage.h>
#include <spargel/base/check.h>
#include <spargel/base/meta.h>
#include <spargel/base/test.h>

using namespace spargel;

TEST(ArrayStorage_DefaultConstructor) {
    base::ArrayStorage<int> y(nullptr);
    spargel_check(y.getCount() == 0);
    spargel_check(y.getAllocator() == nullptr);
}

TEST(ArrayStorage_ConstructByCapacity) {
    base::ArrayStorage<int> x(10, base::default_allocator());
    spargel_check(x.getCount() == 10);
    spargel_check(x.getAllocator() == base::default_allocator());
}

TEST(ArrayStorage_MoveConstructor) {
    base::ArrayStorage<int> x(10, base::default_allocator());
    base::ArrayStorage<int> z(base::move(x));
    spargel_check(x.getCount() == 0);
    spargel_check(z.getCount() == 10);
    spargel_check(z.getAllocator() == base::default_allocator());
}
