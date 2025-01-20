#include <spargel/base/allocator.h>
#include <spargel/base/assert.h>
#include <spargel/base/hash_map.h>

using namespace spargel;

int main() {
    base::HashMap<int, int> x(base::default_allocator());

    spargel_assert(x.count() == 0);
    spargel_assert(x.get(100) == nullptr);

    x.set(1, 2);

    spargel_assert(x.count() == 1);

    spargel_assert(x.get(1) != nullptr);
    spargel_assert(x.get(2) == nullptr);
    spargel_assert(*x.get(1) == 2);

    for (int i = 5; i < 100; i++) {
        x.set(i, i * i);
    }

    for (int i = 5; i < 100; i++) {
        spargel_assert(x.get(i) != nullptr);
        spargel_assert(*x.get(i) == i * i);
    }

    return 0;
}
