#include <spargel/base/assert.h>
#include <spargel/util/path.h>

using namespace spargel;

static bool dirname_equal(const char* path, const char* expected) {
    auto path_dirname = util::dirname(base::String(path));
    return path_dirname == base::String(expected);
}

void test_dirname() {
    // absolute
#if SPARGEL_IS_WINDOWS
    spargel_assert(dirname_equal("C:\\", "C:\\"));
    spargel_assert(dirname_equal("D:\\hello", "D:\\"));
    spargel_assert(dirname_equal("D:\\hello\\", "D:\\"));
    spargel_assert(dirname_equal("C:\\hello\\world", "C:\\hello"));
    spargel_assert(dirname_equal("D:\\hello\\world\\", "D:\\hello"));
#else
    spargel_assert(dirname_equal("/", "/"));
    spargel_assert(dirname_equal("/hello", "/"));
    spargel_assert(dirname_equal("/hello/", "/"));
    spargel_assert(dirname_equal("/hello/world", "/hello"));
    spargel_assert(dirname_equal("/hello/world/", "/hello"));
#endif

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
}

void test_parsePath() {
    util::ParsedPath parsed;

    parsed = util::parsePath(base::String(""));
    spargel_assert(parsed.absolute == false);
    spargel_assert(parsed.directory == false);
    spargel_assert(parsed.components.count() == 0);

    parsed = util::parsePath(base::String("."));
    spargel_assert(parsed.absolute == false);
    spargel_assert(parsed.directory == false);
    spargel_assert(parsed.components.count() == 1);
    spargel_assert(parsed.components[0] == base::String("."));

    parsed = util::parsePath(base::String("./"));
    spargel_assert(parsed.absolute == false);
    spargel_assert(parsed.directory == true);
    spargel_assert(parsed.components.count() == 1);
    spargel_assert(parsed.components[0] == base::String("."));

    parsed = util::parsePath(base::String("hello"));
    spargel_assert(parsed.absolute == false);
    spargel_assert(parsed.directory == false);
    spargel_assert(parsed.components.count() == 1);
    spargel_assert(parsed.components[0] == base::String("hello"));

    parsed = util::parsePath(base::String("hello/"));
    spargel_assert(parsed.absolute == false);
    spargel_assert(parsed.directory = true);
    spargel_assert(parsed.components.count() == 1);
    spargel_assert(parsed.components[0] == base::String("hello"));

    parsed = util::parsePath(base::String("hello/world"));
    spargel_assert(parsed.absolute == false);
    spargel_assert(parsed.directory == false);
    spargel_assert(parsed.components.count() == 2);
    spargel_assert(parsed.components[0] == base::String("hello"));
    spargel_assert(parsed.components[1] == base::String("world"));

    parsed = util::parsePath(base::String("hello/world/"));
    spargel_assert(parsed.absolute == false);
    spargel_assert(parsed.directory = true);
    spargel_assert(parsed.components.count() == 2);
    spargel_assert(parsed.components[0] == base::String("hello"));
    spargel_assert(parsed.components[1] == base::String("world"));

    parsed = util::parsePath(base::String("/hello/world"));
    spargel_assert(parsed.absolute == true);
    spargel_assert(parsed.directory == false);
    spargel_assert(parsed.components.count() == 2);
    spargel_assert(parsed.components[0] == base::String("hello"));
    spargel_assert(parsed.components[1] == base::String("world"));

    parsed = util::parsePath(base::String("/hello/world/"));
    spargel_assert(parsed.absolute == true);
    spargel_assert(parsed.directory = true);
    spargel_assert(parsed.components.count() == 2);
    spargel_assert(parsed.components[0] == base::String("hello"));
    spargel_assert(parsed.components[1] == base::String("world"));
}

int main() {
    test_dirname();

    test_parsePath();

    return 0;
}
