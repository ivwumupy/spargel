#pragma once

#include <spargel/base/either.h>
#include <spargel/base/string_view.h>

namespace spargel::lang {

    enum class TokenKind {
        //------------------------
        // Newline and Whitespace
        //
        // Definition (cf. UTR #13 and UAX #31)
        //  - NLF (new line function) : any of { CR, LF, CRLF, NEL (U+0085) }, depending on the platform
        //  - LS (line separator) : U+2028
        //  - PS (paragraph separator) : U+2029
        //  - Pattern_White_Space characters :
        //      the set { ??? }
        //
        // The syntax is sensitive to line breaks. So newline is separated from whitespace.
        //
        // Decision:
        //  - Only U+000D (CR) and U+000A (LF) are considered as newline.
        //  - Only U+0020 (space) and U+0009 (horizontal tab) are considered as whitespace.
        //
        // Other choices:
        //  - https://go.dev/ref/spec#Tokens
        //  - https://docs.swift.org/swift-book/documentation/the-swift-programming-language/lexicalstructure/#Whitespace-and-Comments
        //
        newline,
        whitespace,

        line_comment,

        identifier,

        left_paren,
        right_paren,
        left_brace,
        right_brace,

        at,
        comma,
        equal,
    };

    struct Token {
        TokenKind kind;
        char const* begin;
        char const* end;

        Token(TokenKind k, char const* b, char const* e) : kind{k}, begin{b}, end{e} {
            spargel_check(b <= e);
        }

        usize length() const { return end - begin; }
    };

    enum class LexError {
        // The cursor has reached the end of the buffer. So there is nothing to lex.
        end_of_buffer,
        internal_error,
    };

    //=============
    // class Lexer
    //
    // A lossless tokenizer.
    //
    class Lexer {
    public:
        using LexResult = base::Either<Token, LexError>;

        Lexer(base::string_view source) : _begin{source.begin()}, _end{source.end()}, _cur{source.begin()} {}

        // Whether the end of buffer is reached.
        bool isEnd() const { return _cur >= _end; }

        // Move forward.
        void advance() {
            if (isEnd()) return;
            _cur++;
        }

        // Get the current byte.
        char peekByte() const {
            if (isEnd()) return 0;
            return *_cur;
        }

        LexResult lex();
        LexResult lexNewline();
        
        void eatWhile(auto&& f) {
            while (!isEnd()) {
                if (!f(peekByte())) return;
                advance();
            }
        }

        // Reset state.
        void reset() { _cur = _begin; }

    private:
        char const* _begin;
        char const* _end;
        char const* _cur;
    };

}
