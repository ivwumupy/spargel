
#include <spargel/base/assert.h>
#include <spargel/util/path.h>

static bool dirname_equal(const char* path, const char* expected) {
    auto path_dirname = spargel::util::dirname(spargel::base::string_from_cstr(path));
    return path_dirname == spargel::base::string_from_cstr(expected);
}

int main() {
    // absolute
#if SPARGEL_IS_WINDOWS

    // TODO
    spargel_assert(dirname_equal("C:\\hello\\world").length() == 8);
    spargel_assert(dirname_equal("D:\\hello\\world\\").length() == 14);

#else  // SPARGEL_IS_WINDOWS

    spargel_assert(dirname_equal("/", "/"));
    spargel_assert(dirname_equal("/hello", "/"));
    spargel_assert(dirname_equal("/hello/", "/"));
    spargel_assert(dirname_equal("/hello/world", "/hello"));
    spargel_assert(dirname_equal("/hello/world/", "/hello"));

#endif  // SPARGEL_IS_WINDOWS

    // relative
    spargel_assert(dirname_equal("", "."));
    spargel_assert(dirname_equal(".", "."));
    spargel_assert(dirname_equal("hello", "."));
    spargel_assert(dirname_equal("hello/", "."));
    spargel_assert(dirname_equal("hello/world", "hello"));
    spargel_assert(dirname_equal("hello/world/", "hello"));
    spargel_assert(dirname_equal("./hello", "."));
    spargel_assert(dirname_equal("./hello/", "."));
    spargel_assert(dirname_equal("./hello/world", "./hello"));
    spargel_assert(dirname_equal("./hello/world/", "./hello"));

    return 0;
}
