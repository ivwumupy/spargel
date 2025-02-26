#include <spargel/base/allocator.h>
#include <spargel/base/assert.h>
#include <spargel/base/hash_map.h>
#include <spargel/base/string.h>

using namespace spargel;

void test_copy() {
    base::HashMap<int, int> z1(base::default_allocator());
    z1.set(2, 100);

    auto z2(z1);
    spargel_assert(z2.get(2) != nullptr);
    spargel_assert(*z2.get(2) == 100);

    auto z3 = z2;
    spargel_assert(z3.get(2) != nullptr);
    spargel_assert(*z3.get(2) == 100);

    base::HashMap<base::string, base::string> z4(base::default_allocator());
    z4.set(base::string("name"), base::string("Alice"));

    auto z5(z4);
    spargel_assert(z5.get(base::string("name")) != nullptr);
    spargel_assert(*z5.get(base::string("name")) == base::string("Alice"));

    auto z6 = z5;
    spargel_assert(z6.get(base::string("name")) != nullptr);
    spargel_assert(*z6.get(base::string("name")) == base::string("Alice"));
}

void test_move() {
    base::HashMap<int, int> z1(base::default_allocator());
    z1.set(2, 100);

    auto z2(base::move(z1));
    spargel_assert(z2.get(2) != nullptr);
    spargel_assert(*z2.get(2) == 100);

    auto z3 = base::move(z2);
    spargel_assert(z3.get(2) != nullptr);
    spargel_assert(*z3.get(2) == 100);

    base::HashMap<base::string, base::string> z4(base::default_allocator());
    z4.set(base::string("name"), base::string("Alice"));

    auto z5(base::move(z4));
    spargel_assert(z5.get(base::string("name")) != nullptr);
    spargel_assert(*z5.get(base::string("name")) == base::string("Alice"));

    auto z6 = base::move(z5);
    spargel_assert(z6.get(base::string("name")) != nullptr);
    spargel_assert(*z6.get(base::string("name")) == base::string("Alice"));
}

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

    test_copy();

    test_move();

    return 0;
}
