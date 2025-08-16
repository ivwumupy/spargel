#pragma once

#include "spargel/base/optional.h"
#include "spargel/base/string_view.h"

namespace spargel::lang {
    enum class TokenKind {
#define TOKEN_KIND(name) name,
#include "spargel/lang/token_kind.inc"
    };
    char const* toStringLiteral(TokenKind kind);
    // Try to convert an identifier into a keyword.
    base::Optional<TokenKind> tryConvertKeyword(base::StringView text);
}  // namespace spargel::lang
