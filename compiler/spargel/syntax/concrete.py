from __future__ import annotations
from dataclasses import dataclass

from spargel.parse.token import Token
from spargel.util.pretty import IndentTracker
from spargel.util.tree import Node, NodeVisitor

class Decl(Node):
    pass

@dataclass
class SourceFile(Node):
    decls: list[Decl]

@dataclass
class OpenDecl(Decl):
    open: Token
    module: ModuleName

@dataclass
class ModuleName:
    name: Token

class Printer(NodeVisitor, IndentTracker):
    def __init__(self):
        super().__init__()
    def iprint(self, s: str):
        """Print with indent"""
        ss = "  " * self.indent_level
        print(ss + s)
    def visit_default(self, node: Node):
        self.iprint(f"<UnknownNode: {node.__class__.__name__}>")
    def visit_SourceFile(self, node: SourceFile):
        self.iprint("SourceFile:")
        with self.indent():
            for decl in node.decls:
                decl.accept(self)
    def visit_OpenDecl(self, node: OpenDecl):
        name = node.module.name.text
        self.iprint(f"OpenDecl: `{name}`")

def dump_node(node: Node):
    printer = Printer()
    node.accept(printer)
