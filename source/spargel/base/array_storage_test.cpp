#include <spargel/base/allocator.h>
#include <spargel/base/array_storage.h>
#include <spargel/base/assert.h>
#include <spargel/base/meta.h>

using namespace spargel;

int main() {
    base::ArrayStorage<int> x(10, base::default_allocator());
    spargel_assert(x.count() == 10);
    base::ArrayStorage<int> y(/*alloc=*/nullptr);
    spargel_assert(y.count() == 0);
    base::ArrayStorage<int> z(base::move(x));
    spargel_assert(x.count() == 0);
    spargel_assert(z.count() == 10);
    return 0;
}
