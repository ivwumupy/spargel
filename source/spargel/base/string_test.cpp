#include <spargel/base/check.h>
#include <spargel/base/string.h>
#include <spargel/base/test.h>

// libc
#include <string.h>

namespace spargel::base {
    namespace {
        using namespace literals;

        bool compare(const String& s, const char* s_expected) {
            usize len = s.length();
            return memcmp(s.data(), s_expected, len) == 0 && s_expected[len] == 0;
        }

        // FIXME
        TEST(String_Empty) {
            String s;
            spargel_check(s.length() == 0);
            spargel_check(s.data() == nullptr);

            String s1, s2;
            spargel_check(s1 == s);
            spargel_check(s2 == s);
            auto s3 = s1 + s2;
            spargel_check(s3 == s);
        }

        TEST(String_Basic) {
            String s = "ABC"_sv;
            spargel_check(s.length() == 3);
            spargel_check(compare(s, "ABC"));
        }

        TEST(String_Add) {
            String s1 = "1"_sv;
            String s2 = "234"_sv;
            String s = s1 + s2;
            spargel_check(s.length() == 4);
            spargel_check(compare(s, "1234"));
        }

        TEST(String_Iterate) {
            String s = "0123456789"_sv;
            for (auto ch : s) {
                spargel_check('0' <= ch && ch <= '9');
            }
        }

        TEST(CString_Basics) {
            char cs[] = {'a', 'b', 'c'};
            CString cstr{cs, cs + 3};
            spargel_check(strcmp(cstr.data(), "abc") == 0);
        }

        TEST(String_CString) {
            String x = "hello world";
            CString cs = x;
            spargel_check(strcmp(cs.data(), "hello world") == 0);
        }
    }  // namespace
}  // namespace spargel::base
