
#include <spargel/base/assert.h>
#include <spargel/util/path.h>

static usize dirname_len(const char* s) {
    auto result = spargel::util::dirname(spargel::base::string_from_cstr(s));
    return result.length();
}

int main() {
    // absolute
#if SPARGEL_IS_WINDOWS

    // TODO
    spargel_assert(dirname_len("C:\\hello\\world").length() == 8);
    spargel_assert(dirname_len("D:\\hello\\world\\").length() == 14);

#else  // SPARGEL_IS_WINDOWS

    spargel_assert(dirname_len("/") == 1);              // "/"
    spargel_assert(dirname_len("/hello") == 1);         // "/"
    spargel_assert(dirname_len("/hello/") == 1);        // "/"
    spargel_assert(dirname_len("/hello/world") == 6);   // "/hello"
    spargel_assert(dirname_len("/hello/world/") == 6);  // "/hello"

#endif  // SPARGEL_IS_WINDOWS

    // relative
    spargel_assert(dirname_len("") == 1);              // "."
    spargel_assert(dirname_len(".") == 1);             // "."
    spargel_assert(dirname_len("hello") == 1);         // "."
    spargel_assert(dirname_len("hello/") == 1);        // "."
    spargel_assert(dirname_len("hello/world") == 5);   // "hello"
    spargel_assert(dirname_len("hello/world/") == 5);  // "hello"

    return 0;
}
