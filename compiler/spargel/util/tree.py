from __future__ import annotations

class Node:
    """Base class for all nodes."""
    def accept(self, visitor: NodeVisitor):
        """Dispatch the visitor."""
        method_name = f"visit_{self.__class__.__name__}"
        visitor_method = getattr(visitor, method_name, visitor.visit_default)
        return visitor_method(self)

class NodeVisitor:
    """Base class for node visitors."""
    def visit_default(self, node: Node):
        pass
