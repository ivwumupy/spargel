#pragma once

#include "spargel/base/either.h"
#include "spargel/base/vector.h"

namespace spargel::lang {
    enum class TokenKind {
        identifier,

        kw_func,
        kw_if,
        kw_open,
        kw_return,

        at_sign,
        colon,
        comma,
        period,
        wildcard,

        left_brace,
        right_brace,
        left_paren,
        right_paren,
        left_square,
        right_square,

        end_of_file,
        unknown,
    };
    class SyntaxToken {
    public:
        TokenKind kind() const;
    };
    enum class SyntaxKind {
        source_file,
        // Open entire module.
        open_decl,
    };
    class SyntaxChild;
    class SyntaxNode {
    public:
        SyntaxKind kind() const { return kind_; }

    private:
        SyntaxKind kind_;
        base::Vector<SyntaxChild> children_;
    };
    class SyntaxChild {
    public:
    private:
        base::Either<SyntaxToken, SyntaxNode> value_;
    };
    class SyntaxVisitor {
    public:
        virtual ~SyntaxVisitor() = default;
    };
}  // namespace spargel::lang
