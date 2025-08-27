#include "spargel/lang/syntax_node.h"

#include "spargel/base/check.h"

namespace spargel::lang {
    void SyntaxNode::dump() {}
    void SyntaxBuilder::pushToken(SyntaxToken token) {
        spargel_check(current_);
        current_->children_.emplace(token);
    }
}  // namespace spargel::lang
