#include <spargel/base/algorithm.h>
#include <spargel/base/assert.h>
#include <spargel/base/logging.h>
#include <spargel/base/meta.h>
#include <spargel/base/optional.h>

using namespace spargel;

struct Foo {
    Foo(int x) { spargel_log_info("Foo()"); }
    ~Foo() { spargel_log_info("~Foo()"); }
};

int main() {
    base::Optional<int> x;
    spargel_assert(!x.hasValue());
    base::Optional<int> y = base::makeOptional<int>(2);
    spargel_assert(y.hasValue());
    spargel_assert(y.value() == 2);
    base::swap(x, y);
    spargel_assert(x.hasValue());
    spargel_assert(!y.hasValue());
    spargel_assert(x.value() == 2);

    base::Optional<Foo> f = base::makeOptional<Foo>(123);
    base::Optional<Foo> g = f;
    base::Optional<Foo> h = base::move(f);
    return 0;
}
