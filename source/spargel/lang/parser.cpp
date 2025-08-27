#include "spargel/lang/parser.h"

#include "spargel/base/check.h"
#include "spargel/base/logging.h"

namespace spargel::lang {
    SyntaxNode* Parser::parse() {
        handleSourceFile();
        return nullptr;
    }
    void Parser::handleSourceFile() {
        beginNode(SyntaxKind::source_file);
        while (true) {
            auto& token = currentToken();
            if (token.kind == TokenKind::end_of_file) {
                break;
            }
            switch (token.kind) {
            case TokenKind::kw_open:
                handleOpenDecl();
                break;
            default:
                spargel_log_error("unknown token");
                token.dump();
                advanceCursor();
                break;
            }
        }
        endNode();
    }
    void Parser::handleOpenDecl() {
        spargel_log_info("open ...");
        beginNode(SyntaxKind::open_decl);
        consumeToken(TokenKind::kw_open);

        endNode();
    }
    void Parser::consumeToken(TokenKind kind) {
        spargel_check(currentToken().kind == kind);
        builder_.pushToken(currentToken());
        advanceCursor();
    }
}  // namespace spargel::lang
