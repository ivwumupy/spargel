#pragma once

namespace spargel::lang {
    enum class SyntaxKind {
        SourceFile,
    };
    class SyntaxNode {
    public:
        virtual ~SyntaxNode() = default;

        SyntaxKind kind() const { return kind_; }

    protected:
        explicit SyntaxNode(SyntaxKind kind) : kind_{kind} {}

    private:
        SyntaxKind kind_;
    };
    class SyntaxVisitor {
    public:
        virtual ~SyntaxVisitor() = default;
    };
}  // namespace spargel::lang
