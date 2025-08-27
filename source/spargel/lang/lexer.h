#pragma once

#include "spargel/base/string_view.h"
#include "spargel/lang/syntax_token.h"

namespace spargel::lang {
    class Cursor {
    public:
        Cursor(char const* begin, char const* end) : input_{begin}, end_{end} {}
        Cursor(base::StringView s) : Cursor{s.begin(), s.end()} {}

        SyntaxToken nextToken();

    private:
        bool isAtEnd() const { return input_ >= end_; }

        // Return '\0' if out-of-bound.
        char peek() const {
            if (isAtEnd()) {
                return 0;
            }
            return *input_;
        }
        void advance() {
            if (input_ >= end_) {
                return;
            }
            input_++;
        }

        template <typename F>
        void advanceWhile(F&& f) {
            while (!isAtEnd()) {
                if (!f(peek())) {
                    break;
                }
                advance();
            }
        }

        SyntaxToken makeToken(char const* start, TokenKind kind) {
            return SyntaxToken{kind, base::StringView{start, input_}};
        }

        SyntaxToken handleWhitespace();
        SyntaxToken handleIdentifierOrKeyword();

        char const* input_;
        char const* end_;
    };
    class Lexer {
    public:
    private:
        base::StringView source_;
    };
}  // namespace spargel::lang
