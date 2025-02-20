#include <spargel/base/logging.h>
#include <spargel/base/sum_type.h>

struct Foo {
    ~Foo() { spargel_log_info("~Foo();"); }
};

int main() {
    auto x = spargel::base::SumType<int, double, int, Foo>::make<2>(3);
    spargel_log_info("x.getValue<2>() = %d", x.getValue<2>());
    x.getValue<2>() = 100;
    spargel_log_info("x.getValue<2>() = %d", x.getValue<2>());
    // spargel_log_info("x.get<1>() = %.3f", x.get<1>());
    // spargel_log_info("x.get<2>() = %d", x.get<2>());
    // x.setIndex(3);
    // x.reset();
    return 0;
}
