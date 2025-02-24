#include <spargel/base/algorithm.h>
#include <spargel/base/assert.h>
#include <spargel/base/logging.h>
#include <spargel/base/meta.h>
#include <spargel/base/optional.h>
#include <spargel/base/unique_ptr.h>

using namespace spargel;

struct Foo {
    Foo(int x) { spargel_log_info("Foo()"); }
    ~Foo() { spargel_log_info("~Foo()"); }
};

struct Base {};

struct Derived : Base {};

int main() {
    base::unique_ptr<Derived> pd = base::make_unique<Derived>();
    base::Optional<base::unique_ptr<Derived>> od =
        base::makeOptional<base::unique_ptr<Derived>>(base::move(pd));
    base::Optional<base::unique_ptr<Base>> b = base::move(od);
        // base::makeOptional<base::unique_ptr<Base>>(base::move(pd));

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
