#include <spargel/base/check.h>
#include <spargel/base/string.h>
#include <spargel/base/test.h>

// libc
#include <string.h>

using namespace spargel::base;
using namespace spargel::base::literals;

namespace {

    bool compare(const string& s, const char* s_expected) {
        return strcmp(s.data(), s_expected) == 0;
    }

}  // namespace

/* FIXME
TEST(String_Empty) {
    string s;
    spargel_check(s.length() == 0);
    spargel_check(s.data() == nullptr);
    // spargel_check(*s.data() == '\0');

    string s1, s2;
    spargel_check(s1 == s);
    spargel_check(s2 == s);
    auto s3 = s1 + s2;
    spargel_check(s3 == s);
}
*/

TEST(String_Basic) {
    {
        string s = "ABC"_sv;
        spargel_check(s.length() == 3);
        spargel_check(compare(s, "ABC"));
    }
}

TEST(String_Add) {
    {
        string s1 = "1"_sv;
        string s2 = "234"_sv;
        string s = s1 + s2;
        spargel_check(s.length() == 4);
        spargel_check(compare(s, "1234"));
    }
}

TEST(String_Iterate) {
    {
        string s = "0123456789"_sv;
        for (auto ch : s) {
            spargel_check('0' <= ch && ch <= '9');
        }
    }
}

TEST(String_Copy_Zero) {
    {
        // fill the heap with data
        vector<char> v;
        for (int i = 0; i < 10000; i++) {
            v.push('0');
        }
    }
    {
        string s1("ABC");
        spargel_check(s1.data()[3] == '\0');
        string s2(s1);
        spargel_check(s2.data()[3] == '\0');
    }
}
