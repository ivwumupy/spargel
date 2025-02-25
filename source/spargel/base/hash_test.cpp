#include <spargel/base/hash.h>
#include <spargel/base/logging.h>
#include <spargel/base/platform.h>
#include <spargel/base/string.h>

using namespace spargel;

constexpr u8 data[] = {0, 1, 2, 3, 4, 5};
// constexpr u64 result =
//     base::__wyhash::wyhash(data, sizeof(data), base::__wyhash::default_seed);

struct Foo {
    u8 x;
    u32 y;
    base::string s;
};

void tag_invoke(base::tag<base::hash>, base::HashRun& run, Foo const& f) {
    run.combine(f.x);
    run.combine(f.y);
    run.combine(f.s);
}

int main() {
    auto r = base::HashRun();
    base::hash(r, base::get_executable_path());
    spargel_log_info("%llu", r.result());
    spargel_log_info("%llu", base::hash(base::get_executable_path()));
    // spargel_log_info("%llu", result);

    Foo f;
    f.x = 1;
    f.y = 2;
    f.s = base::string("hello");

    spargel_log_info("%llu", base::hash(f));

    f.s = base::string("world");
    spargel_log_info("%llu", base::hash(f));

    return 0;
}
