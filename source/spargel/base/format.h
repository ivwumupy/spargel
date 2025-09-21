#pragma once

#include "spargel/base/tag_invoke.h"

namespace spargel::base {
    // TODO
    template <typename T>
    concept FormatTarget = true;

    namespace detail {
        // TODO: Type-safe `FormatString`.
        class FormatString {
        public:
            constexpr FormatString(char const* s)
                : begin_{s}, end_{s + __builtin_strlen(s)} {}

            constexpr char const* begin() const { return begin_; }
            constexpr char const* end() const { return end_; }

        private:
            char const* begin_;
            char const* end_;
        };
    }  // namespace detail

    namespace format_ {
        struct ParseFormatCPO {
            template <typename T>
            constexpr decltype(auto) operator()(T&& t) const {
                return tag_invoke(ParseFormatCPO{}, base::forward<T>(t));
            }
        };
    }  // namespace format_

    // `parseFormat` is a customization-point to handle the parsing of the
    // format specification. It should return a formatter object that will be
    // invoked to perform the format.
    inline constexpr format_::ParseFormatCPO parseFormat;

    template <FormatTarget Target, typename... Args>
    constexpr void formatTo(Target& target, detail::FormatString fmt,
                            Args&&... args) {
        auto it = fmt.begin();
        auto fragment_begin = it;
        while (it < fmt.end()) {
            auto c = *it;
            if (c != '{') {
                it++;
                continue;
            }
        }
    }

    namespace detail {
        struct ConsoleTarget {
            // TODO
        };
    }  // namespace detail

    template <typename... Args>
    void print(detail::FormatString fmt, Args&&... args) {
        detail::ConsoleTarget target;
        formatTo(target, fmt, base::forward<Args>(args)...);
    }
}  // namespace spargel::base
