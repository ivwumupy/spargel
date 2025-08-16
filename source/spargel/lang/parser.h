#pragma once

#include "spargel/lang/lexer.h"
#include "spargel/lang/syntax_node.h"

namespace spargel::lang {
    class Parser {
    public:
        explicit Parser(Cursor cursor) : cursor_{cursor} {}

        SyntaxNode* parse();

    private:
        void handleSourceFile();
        void handleOpenDecl();
        void handleFuncDecl();

        Cursor cursor_;
    };
}  // namespace spargel::lang
