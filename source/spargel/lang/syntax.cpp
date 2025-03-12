#include <spargel/base/check.h>
#include <spargel/lang/syntax.h>

namespace spargel::lang {

    namespace {

        bool isNewline(char c) { return c == '\n' || c == '\r'; }

    }

    Lexer::LexResult Lexer::lex() {
        if (isEnd()) return base::Right{LexError::end_of_buffer};

        char c = peekByte();

        switch (c) {

        // newline
        case '\n':
        case '\r':
            return lexNewline();

        default:
            break;
        }

        return base::Right{LexError::internal_error};
    }

    Lexer::LexResult Lexer::lexNewline() {
        spargel_check(isNewline(peekByte()));

        char const* begin = _cur;
        eatWhile([](char c) {
            return c == '\n' || c == '\r';
        });
        return base::Left{Token(TokenKind::newline, begin, _cur)};
    }

}
