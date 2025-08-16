#pragma once

#include "spargel/base/either.h"
#include "spargel/base/vector.h"
#include "spargel/lang/syntax_token.h"

namespace spargel::lang {
    enum class SyntaxKind {
        source_file,
        open_decl,
        func_decl,
    };
    class SyntaxChild;
    class SyntaxNode {
    public:
        SyntaxKind kind() const { return kind_; }

        void push(SyntaxToken token) { children_.emplace(token); }
        void push(SyntaxNode node) { children_.emplace(node); }

    private:
        SyntaxKind kind_;
        base::Vector<SyntaxChild> children_;
    };
    class SyntaxChild {
    public:
        SyntaxChild(SyntaxToken token) : value_{base::Left{token}} {}
        SyntaxChild(SyntaxNode node) : value_{base::Right{node}} {}

        bool isToken() const { return value_.isLeft(); }
        bool isNode() const { return value_.isRight(); }

        SyntaxToken& asToken() { return value_.left(); }
        SyntaxNode& asNode() { return value_.right(); }

    private:
        base::Either<SyntaxToken, SyntaxNode> value_;
    };
    class SyntaxBuilder {
    public:
    };
}  // namespace spargel::lang
