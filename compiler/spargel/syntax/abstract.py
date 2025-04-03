from __future__ import annotations
from dataclasses import dataclass

from spargel.util.pretty import IndentTracker
from spargel.util.tree import Node, NodeVisitor

class Name:
    """Abstract name"""
    value: int

class Lit(Node):
    """Literal"""

class Expr(Node):
    """Expression"""

class Decl(Node):
    """Declaration"""

class Stmt(Node):
    """Statement"""

@dataclass
class SourceFile(Node):
    decls: list[Decl]

@dataclass
class FuncDecl(Decl):
    """Function"""

@dataclass
class ModuleName:
    value: str

@dataclass
class OpenDecl(Decl):
    """Open"""
    module: ModuleName

@dataclass
class LitInt(Lit):
    value: int

@dataclass
class LitStr(Lit):
    value: str

class AbstractNodeVisitor(NodeVisitor):
    def visit_default(self, node: Node):
        pass
    def visit_SourceFile(self, node: SourceFile):
        for decl in node.decls:
            decl.accept(self)
    def visit_FuncDecl(self, node: FuncDecl):
        pass
    def visit_OpenDecl(self, node: OpenDecl):
        pass
    def visit_LitInt(self, node: LitInt):
        pass
    def visit_LitStr(self, node: LitStr):
        pass

class Printer(AbstractNodeVisitor, IndentTracker):
    def __init__(self):
        super().__init__()
    def iprint(self, s: str):
        """Print with indent"""
        ss = "  " * self.indent_level
        print(ss + s)
    def visit_SourceFile(self, node: SourceFile):
        self.iprint("SourceFile")
        with self.indent():
            super().visit_SourceFile(node)
    def visit_OpenDecl(self, node: OpenDecl):
        module_name = node.module.value
        self.iprint(f"OpenDecl: `{module_name}`")
    def visit_FuncDecl(self, node: FuncDecl):
        self.iprint("FuncDecl")
    def visit_LitInt(self, node: LitInt):
        self.iprint(f"LitInt({node.value})")
