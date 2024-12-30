#include <spargel/base/hash.h>
#include <spargel/base/logging.h>
#include <spargel/base/platform.h>
#include <spargel/base/string.h>

constexpr u8 data[] = {0, 1, 2, 3, 4, 5};
constexpr u64 result =
    spargel::base::__wyhash::wyhash(data, sizeof(data), spargel::base::__wyhash::default_seed);

struct Foo {
    u8 x;
    u32 y;
    spargel::base::string s;
};

void tag_invoke(spargel::base::tag<spargel::base::hash>, spargel::base::HashRun& run,
                Foo const& f) {
    run.combine(f.x);
    run.combine(f.y);
    run.combine(f.s);
}

int main() {
    auto r = spargel::base::HashRun();
    spargel::base::hash(r, spargel::base::get_executable_path());
    spargel_log_info("%llu", r.result());
    spargel_log_info("%llu", spargel::base::hash(spargel::base::get_executable_path()));
    spargel_log_info("%llu", result);

    Foo f;
    f.x = 1;
    f.y = 2;
    f.s = spargel::base::string("hello");

    spargel_log_info("%llu", spargel::base::hash(f));

    f.s = spargel::base::string("world");
    spargel_log_info("%llu", spargel::base::hash(f));

    return 0;
}
