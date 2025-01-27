#pragma once

#include <spargel/base/allocator.h>
#include <spargel/base/assert.h>
#include <spargel/base/hash.h>
#include <spargel/base/string_view.h>
#include <spargel/base/tag_invoke.h>
#include <spargel/base/types.h>
#include <spargel/base/vector.h>

namespace spargel::base {

    namespace _string {
        struct string {
            string() = default;

            string(string_view v);

            string(string const& other);
            string& operator=(string const& other);

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

            friend void tag_invoke(tag<hash>, HashRun& r, string const& s) {
                r.combine((u8 const*)s._data, s._length);
            }

            usize _length = 0;
            char* _data = nullptr;
        };

        class UnicodeScalar {};

        class String {
        public:
            String(Allocator* alloc) : _alloc{alloc} { spargel_assert(alloc != nullptr); }

            /// Get the `i`-th byte.
            u8& operator[](usize i) { return _bytes[i]; }
            u8 const& operator[](usize i) const { return _bytes[i]; }

            span<u8> bytes() const { return _bytes.toSpan(); }

            // get the unicode scalar containing the |i|-th byte
            u32 getScalarAtByte(usize i) {
                spargel_assert(i < _bytes.count());
                u8 byte = _bytes[i];
                if ((byte & 0b10000000) == 0) {
                    // 0xxxxxxx, single byte
                    return byte;
                } else if ((byte & 0b11100000) == 0b1100000) {
                    // 110yyyyy, first byte of two byte
                    return (((u32)byte) << 8) | _bytes[i + 1];
                } else if ((byte & 0b11110000) == 0b11100000) {
                    return 0;
                } else if ((byte & 0b11111000) == 0b11110000) {
                    return 0;
                }
                spargel_panic_here();
            }

            // unsafe
            void appendByte(u8 b) { _bytes.push(b); }

        private:
            vector<u8> _bytes;
            [[maybe_unused]] Allocator* _alloc;
        };

    }  // namespace _string

    using _string::string;
    using _string::String;

    string string_from_cstr(char const* str);

    string string_from_range(char const* begin, char const* end);

    string string_concat(string const& str1, string const& str2);

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
