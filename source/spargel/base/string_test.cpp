#include <spargel/base/assert.h>
#include <spargel/base/string.h>

using namespace spargel;
using namespace spargel::base;

void test_emptyString() {
    string s;
    spargel_assert(s.length() == 0);
    spargel_assert(s.data() != nullptr);
    spargel_assert(*s.data() == '\0');

    string s1, s2;
    spargel_assert(s1 == s);
    spargel_assert(s2 == s);
    auto s3 = s1 + s2;
    spargel_assert(s3 == s);
}

int main() {
    test_emptyString();

    return 0;
}
