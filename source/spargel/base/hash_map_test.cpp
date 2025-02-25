#include <spargel/base/allocator.h>
#include <spargel/base/assert.h>
#include <spargel/base/hash_map.h>
#include <spargel/base/string.h>

#include <cassert>

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

    base::HashMap<base::string, base::string> y(base::default_allocator());

    spargel_assert(y.count() == 0);
    spargel_assert(y.get(base::string("name")) == nullptr);

    y.set(base::string("name"), base::string("Alice"));

    spargel_assert(y.count() == 1);
    spargel_assert(*y.get(base::string("name")) == base::string("Alice"));

    // copy
    x.set(1, 10);
    auto z = x;
    assert(z.count() == x.count());
    assert(z.get(1) != x.get(1));
    assert(*z.get(1) == *x.get(1));
    z.set(1, 20);
    assert(*z.get(1) != *x.get(1));

    return 0;
}
