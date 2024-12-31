#include <spargel/base/logging.h>
#include <spargel/base/sum_type.h>

struct Foo {
    ~Foo() { spargel_log_info("~Foo();"); }
};

int main() {
    spargel::base::SumType<int, double, int, Foo> x;
    x.get<0>() = 100;
    spargel_log_info("x.get<0>() = %d", x.get<0>());
    spargel_log_info("x.get<1>() = %.3f", x.get<1>());
    spargel_log_info("x.get<2>() = %d", x.get<2>());
    x.setIndex(3);
    x.reset();
    return 0;
}
