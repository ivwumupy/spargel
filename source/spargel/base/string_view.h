#pragma once

#include "spargel/base/check.h"
#include "spargel/base/format.h"
#include "spargel/base/hash.h"
#include "spargel/base/types.h"

// libc
#include <string.h>

namespace spargel::base {

    namespace string_view_ {
        class StringView {
        public:
            constexpr StringView() = default;
            constexpr StringView(char const* begin, char const* end)
                : _begin{begin}, _end{end} {}
            constexpr StringView(char const* begin, usize len)
                : _begin{begin}, _end{begin + len} {}
            constexpr StringView(char const* s)
                : StringView{s, __builtin_strlen(s)} {}

            constexpr char operator[](usize i) const { return _begin[i]; }
            constexpr usize length() const {
                spargel_check(_begin <= _end);
                return static_cast<usize>(_end - _begin);
            }
            constexpr char const* begin() const { return _begin; }
            constexpr char const* end() const { return _end; }
            constexpr char const* data() const { return _begin; }

            friend bool operator==(StringView const& lhs,
                                   StringView const& rhs) {
                if (lhs.length() != rhs.length()) return false;
                return memcmp(lhs._begin, rhs._begin, lhs.length()) == 0;
            }

            friend void tag_invoke(tag<hash>, HashRun& run,
                                   StringView const& self) {
                run.combine(reinterpret_cast<u8 const*>(self._begin),
                            self.length());
            }

        private:
            char const* _begin = nullptr;
            char const* _end = nullptr;
        };

        template <FormatTarget Target>
        void tag_invoke(tag<formatArg>, Target& target, detail::FormatString,
                        StringView sv) {
            target.append(sv.begin(), sv.end());
        }
    }  // namespace string_view_

    using string_view_::StringView;
    using string_view = string_view_::StringView;

    namespace literals {
        inline constexpr StringView operator""_sv(char const* begin,
                                                  usize len) {
            return StringView(begin, len);
        }
    }  // namespace literals

}  // namespace spargel::base
