#pragma once

#include <spargel/base/hash.h>
#include <spargel/base/types.h>

// libc
#include <string.h>

namespace spargel::base {

    namespace string_view_ {
        class string_view {
        public:
            constexpr string_view() = default;
            constexpr string_view(char const* begin, char const* end) : _begin{begin}, _end{end} {}
            constexpr string_view(char const* begin, usize len)
                : _begin{begin}, _end{begin + len} {}
            // template <usize N>
            // constexpr string_view(char const (&str)[N]) : string_view(str, str + N - 1) {}

            constexpr char operator[](usize i) const { return _begin[i]; }
            constexpr usize length() const { return _end - _begin; }
            constexpr char const* begin() const { return _begin; }
            constexpr char const* end() const { return _end; }
            constexpr char const* data() const { return _begin; }

            friend bool operator==(string_view const& lhs, string_view const& rhs) {
                if (lhs.length() != rhs.length()) return false;
                return memcmp(lhs._begin, rhs._begin, lhs.length()) == 0;
            }

            friend void tag_invoke(tag<hash>, HashRun& run, string_view const& self) {
                run.combine(reinterpret_cast<u8 const*>(self._begin), self.length());
            }

        private:
            char const* _begin = nullptr;
            char const* _end = nullptr;
        };
    }  // namespace string_view_

    using StringView = string_view_::string_view;
    using string_view = StringView;

    namespace literals {
        inline constexpr StringView operator""_sv(char const* begin, usize len) {
            return StringView(begin, len);
        }
    }  // namespace literals

}  // namespace spargel::base
