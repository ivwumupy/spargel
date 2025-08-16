#include "spargel/lang/token_kind.h"

namespace spargel::lang {
    char const* toStringLiteral(TokenKind kind) {
        switch (kind) {
#define TOKEN_KIND(name)  \
    case TokenKind::name: \
        return #name;
#include "spargel/lang/token_kind.inc"
        }
    }
    base::Optional<TokenKind> tryConvertKeyword(base::StringView text) {
#define KEYWORD_TOKEN_KIND(name, t)                            \
    if (text == #t) {                                          \
        return base::makeOptional<TokenKind>(TokenKind::name); \
    }
#include "spargel/lang/token_kind.inc"
        return base::nullopt;
    }
}  // namespace spargel::lang
