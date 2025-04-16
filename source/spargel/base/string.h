#pragma once

#include <spargel/base/algorithm.h>
#include <spargel/base/allocator.h>
#include <spargel/base/assert.h>
#include <spargel/base/check.h>
#include <spargel/base/hash.h>
#include <spargel/base/string_view.h>
#include <spargel/base/tag_invoke.h>
#include <spargel/base/types.h>
#include <spargel/base/vector.h>

// libc
#include <string.h>

namespace spargel::base {

    namespace _string {
        struct string {
            string() = default;

            string(string_view v);

            string(string const& other);
            string& operator=(string const& other) {
                if (this != &other) {
                    string tmp(other);
                    swap(*this, tmp);
                }
                return *this;
            }

            string(string&& other) { swap(*this, other); }
            string& operator=(string&& other) {
                if (this != &other) {
                    string tmp(move(other));
                    swap(*this, tmp);
                }
                return *this;
            }

            explicit string(const char* cstr);
            string& operator=(const char* cstr) {
                string tmp(cstr);
                swap(*this, tmp);
                return *this;
            }

            explicit string(char ch);
            string& operator=(char ch) {
                string tmp(ch);
                swap(*this, tmp);
                return *this;
            }

            ~string();

            char& operator[](usize i) { return _data[i]; }
            char const& operator[](usize i) const { return _data[i]; }

            usize length() const { return _length; }
            char* begin() { return _data; }
            char const* begin() const { return _data; }
            char* end() { return _data + _length; }
            char const* end() const { return _data + _length; }
            char* data() { return _data; }
            char const* data() const { return _data; }
            string_view view() const { return string_view(_data, _data + _length); }

            friend bool operator==(string const& lhs, string const& rhs);

            friend string operator+(const string& lhs, const string& rhs);

            friend string operator+(const string& s, char ch);

            friend string operator+(const string& s, const char* s2) { return s + string(s2); }

            friend void tag_invoke(tag<swap>, string& lhs, string& rhs) {
                swap(lhs._length, rhs._length);
                swap(lhs._data, rhs._data);
            }

            friend void tag_invoke(tag<hash>, HashRun& r, string const& s) {
                r.combine((u8 const*)s._data, s._length);
            }

            usize _length = 0;
            char* _data = nullptr;
        };

        // clang-format off
        /// A code point is a value in the Unicode codespace, i.e. an integer in the range [0, 10ffff].
        /// A Unicode scalar value is a code point that is not a high-surrogate or a low-surrogate.
        /// In other words, a Unicode scalar value is a code point in the range [0, d7ff] union [e000, 10ffff].
        ///
        /// A `UnicodeScalar` represents a Unicode scalar value.
        ///
        // clang-format on
        class UnicodeScalar {
        public:
            UnicodeScalar() = default;

            UnicodeScalar(u32 value) : _value{value} { spargel_assert(isValid(_value)); }

            UnicodeScalar(UnicodeScalar const& other) : _value{other._value} {}
            UnicodeScalar& operator=(UnicodeScalar const& other) {
                _value = other._value;
                return *this;
            }

            u32 getValue() const { return _value; }

        private:
            static bool isValid(u32 value) {
                return (value <= 0x10ffff) && ((value <= 0xd7ff) || (value >= 0xe000));
            }

            u32 _value = 0;
        };

        /// A UTF-8 string.
        ///
        ///-------
        /// UTF-8
        ///
        /// 0yyyzzzz
        /// 110xxxyy 10yyzzzz
        /// 1110wwww 10xxxxyy 10yyzzzz
        /// 11110uvv 10vvwwww 10xxxxyy 10yyzzzz
        ///
        class String {
        public:
            static String from_range(char const* begin, char const* end) {
                usize len = end - begin;
                String result;
                result._bytes.reserve(len + 1);
                result._bytes.set_count(len);
                memcpy(result._bytes.data(), begin, len);
                result._bytes.data()[len] = 0;
                return result;
            }

            String(Allocator* alloc = default_allocator()) : _bytes(alloc), _alloc{alloc} {
                spargel_assert(alloc != nullptr);
            }

            String(String const& other) : _bytes(other._bytes), _alloc{other._alloc} {}
            String& operator=(String const& other) {
                String tmp(other);
                swap(*this, tmp);
                return *this;
            }

            String(String&& other) : _bytes(base::move(other._bytes)), _alloc{other._alloc} {}
            String& operator=(String&& other) {
                String tmp(base::move(other));
                swap(*this, tmp);
                return *this;
            }

            ~String() = default;

            // migration from base::string
            /*explicit*/ String(StringView view) : _alloc{default_allocator()} {
                usize len = view.length();
                _bytes.reserve(len + 1);
                _bytes.set_count(len);
                memcpy(_bytes.data(), view.data(), len);
                _bytes.data()[len] = 0;
            }
            explicit String(char const* cstr) : _alloc{default_allocator()} {
                usize len = strlen(cstr);
                _bytes.reserve(len + 1);
                _bytes.set_count(len);
                memcpy(_bytes.data(), cstr, len);
                _bytes.data()[len] = 0;
            }
            String& operator=(char const* cstr) {
                String tmp(cstr);
                swap(*this, tmp);
                return *this;
            }
            explicit String(char ch) : _alloc{default_allocator()} {
                _bytes.reserve(2);
                _bytes.set_count(1);
                _bytes.data()[0] = ch;
                _bytes.data()[1] = 0;
            }
            String& operator=(char ch) {
                String tmp(ch);
                swap(*this, tmp);
                return *this;
            }
            char& operator[](usize i) { return _bytes[i]; }
            char const& operator[](usize i) const { return _bytes[i]; }

            usize length() const { return _bytes.count(); }
            char* begin() { return _bytes.begin(); }
            char const* begin() const { return _bytes.begin(); }
            char* end() { return _bytes.end(); }
            char const* end() const { return _bytes.end(); }
            char* data() { return _bytes.data(); }
            char const* data() const { return _bytes.data(); }
            StringView view() const { return StringView(begin(), end()); }
            friend bool operator==(String const& lhs, String const& rhs) {
                if (lhs.length() != rhs.length()) return false;
                return memcmp(lhs.data(), rhs.data(), lhs.length()) == 0;
            }
            friend String operator+(String const& lhs, String const& rhs) {
                spargel_check(lhs._alloc == rhs._alloc);
                usize len = lhs.length() + rhs.length();
                String result(lhs._alloc);
                result._bytes.reserve(len + 1);
                result._bytes.set_count(len);
                memcpy(result._bytes.data(), lhs.data(), lhs.length());
                memcpy(result._bytes.data() + lhs.length(), rhs.data(), rhs.length());
                result._bytes.data()[len] = 0;
                return result;
            }
            // not efficient
            friend String operator+(String const& s, char ch) {
                return s + String(ch);
            }
            friend String operator+(String const& s, char const* s2) { return s + String(s2); }

            /// Get the `i`-th byte.
            Byte getByte(usize i) const { return (Byte)_bytes[i]; }

            span<Byte> bytes() const { return _bytes.toSpan().asBytes(); }

            usize getLength() const {
                usize len = 0;
                usize i = 0;
                while (i < _bytes.count()) {
                    Byte byte = _bytes[i];
                    if ((byte & 0b10000000) == 0) {
                        i += 1;
                    } else if ((byte & 0b11100000) == 0b11000000) {
                        i += 2;
                    } else if ((byte & 0b11110000) == 0b11100000) {
                        i += 3;
                    } else if ((byte & 0b11111000) == 0b11110000) {
                        i += 4;
                    } else {
                        spargel_panic_here();
                    }
                    len += 1;
                }
                // spargel_assert(i == _bytes.count());
                return len;
            }

            /// Get the unicode scalar containing the `i`-th byte.
            // u32 getScalarAtByte(usize i) {
            //     spargel_panic_here();
            // }

            // unsafe
            void appendByte(Byte b) { _bytes.push((char)b); }

            Allocator* getAllocator() { return _alloc; }

            friend void tag_invoke(tag<swap>, String& lhs, String& rhs) {
                swap(lhs._bytes, rhs._bytes);
                swap(lhs._alloc, rhs._alloc);
            }

            friend void tag_invoke(tag<hash>, HashRun& r, String const& s) {
                r.combine((u8 const*)s._bytes.data(), s.length());
            }

        private:
            vector<char> _bytes;
            Allocator* _alloc;
        };

    }  // namespace _string

    // using _string::string;
    using _string::String;
    using string = String;

    String string_from_range(char const* begin, char const* end);
    // string string_from_range(char const* begin, char const* end);

}  // namespace spargel::base

// # Unicode
//
// ## Glossary
//
// - code point: any value in the unicode codespace, i.e. the range of integers from 0 to 10ffff
// - unicode scalar value; any unicode code point except high-surrogate and low-surrogate code
// points, i.e. [0, d7ff] cup [e000, 10ffff]
//
// - utf-8 encoding form: the unicode encoding form that assigns each unicode scalar value to an
// unsigned byte sequence of one to four bytes in length.
//
// - extended grapheme cluster: the text between extended grapheme cluster boundaries as specified
// by unicode standard annex #29
//
