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
        explicit SyntaxNode(SyntaxKind kind) : kind_{kind} {}

        SyntaxKind kind() const { return kind_; }

        base::Span<SyntaxChild> children() const { return children_.toSpan(); }

        void dump();

    private:
        friend class SyntaxBuilder;

        SyntaxKind kind_;
        base::Vector<SyntaxChild> children_;
    };
    class SyntaxChild {
    public:
        SyntaxChild(SyntaxToken token) : value_{base::Left{token}} {}
        SyntaxChild(SyntaxKind kind) : value_{base::Right{SyntaxNode{kind}}} {}

        bool isToken() const { return value_.isLeft(); }
        bool isNode() const { return value_.isRight(); }

        SyntaxToken& asToken() { return value_.left(); }
        SyntaxNode& asNode() { return value_.right(); }

    private:
        base::Either<SyntaxToken, SyntaxNode> value_;
    };
    class SyntaxBuilder {
    public:
        // Start building a new child node.
        void beginNode([[maybe_unused]] SyntaxKind kind) {}
        // Finish the current child.
        void endNode() {}
        void pushToken(SyntaxToken token);

    private:
        SyntaxNode* current_ = nullptr;
    };
}  // namespace spargel::lang
