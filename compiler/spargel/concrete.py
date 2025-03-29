"""Concrete Syntax Tree
"""

from dataclasses import dataclass

import spargel.lexer as L

@dataclass
class SourceFile:
    """A source file.

    Args:
        items -- module items
        eof_tok -- token for end of file
    """
    items: "[ModuleItem]"
    eof_tok: "Token"

@dataclass
class ModuleItem:
    """An item in a module.
    """
    item: "OpenDecl | FuncDecl | ImplDecl"

@dataclass
class OpenDecl:
    """An open declaration.
    """
    open_tok: "Token"
    path: "Token"
    #path: "ModulePath"

@dataclass
class ModulePath:
    """The path of a module.
    """
    segments: "[ModuleName]"

@dataclass
class ModuleName:
    token: "Token"

@dataclass
class FuncDecl:
    """A function declaration.
    """
    func_tok: "Token"
    name: "Token"
    func_sig: "FuncSig"

@dataclass
class FuncSig:
    """Function signature
    """
    lparen_tok: "Token"
    params: "FuncParams"
    rparen_tok: "Token"

@dataclass
class FuncParams:
    """Function parameters
    """
    params: "[FuncParam]"

@dataclass
class FuncParam:
    """Function parameter
    """
    name: "Token"
    colon_tok: "Token?"
    type: "Expr?"
    comma_tok: "Token?"

@dataclass
class GroupedExpr:
    """A parenthesized expression
    """
    lparen_tok: "Token"
    expr: "Expr"
    rparen_tok: "Token"

@dataclass
class LitExpr:
    """Literal expression
    """
    token: "Token"

@dataclass
class IdentExpr:
    """Identifier expression
    """
    token: "Token"

@dataclass
class AddExpr:
    left: "Expr"
    add_tok: "Token"
    right: "Expr"

@dataclass
class CallExpr:
    func: "Expr"
    lparen_tok: "Token"
    params: "CallParams"
    rparen_tok: "Token"

@dataclass
class CallParams:
    params: "[CallParam]"

@dataclass
class CallParam:
    expr: "Expr"

@dataclass
class ImplDecl:
    impl_tok: "Token"
    name: "Token"
    lbrace_tok: "Token"
    funcs: "[FuncDecl]"
    rbrace_tok: "Token"

def short_desc(node):
    match node:
        case SourceFile(items, eof_tok):
            n = len(items)
            return f"SourceFile [{n} items]"
        case ModuleItem():
            return "ModuleItem"
        case OpenDecl(open_tok, path):
            return f"OpenDecl <path = {path.text}>"
        case FuncDecl(func_tok, name):
            return f"FuncDecl <name = {name.text}>"
        case FuncSig(lparen_tok, params, rparen_tok):
            # TODO
            return "FuncSig [() -> Unit]"
        case FuncParams(params):
            n = len(params)
            return f"FuncParams [{n} params]"
        case FuncParam(name, colon_tok, type):
            return f"FuncParam <name = {name.text}>"
        case AddExpr(left, add_tok, right):
            return f"AddExpr"
        case LitExpr(tok):
            return f"Literal <value = {tok.text}>"
        case GroupedExpr(lparen_tok, expr, rparen_tok):
            return "GroupedExpr"
        case CallExpr(func, lparen_tok, params, rparen_tok):
            return "CallExpr"
        case CallParams(params):
            return "CallParams"
        case CallParam(expr):
            return "CallParam"
        case IdentExpr(tok):
            return f"IdentExpr <name = {tok.text}>"
        case _:
            return "<Unknown>"

def dump_children(node, level):
    match node:
        case SourceFile(items, eof):
            dump_array(items, level)
        case ModuleItem(item):
            dump_node(item, level)
        case FuncDecl(func_tok, name, func_sig):
            dump_node(func_sig, level)
        case FuncSig(lparen_tok, params, rparen_tok):
            dump_node(params, level)
        case FuncParams(params):
            dump_array(params, level)
        case FuncParam(name, colon_tok, type):
            if type is not None:
                dump_node(type, level)
        case AddExpr(left, add_tok, right):
            dump_node(left, level)
            dump_node(right, level)
        case GroupedExpr(lparen_tok, expr, rparen_tok):
            dump_node(expr, level)
        case CallExpr(func, lparen_tok, params, rparen_tok):
            dump_node(func, level)
            dump_node(params, level)
        case CallParams(params):
            dump_array(params, level)
        case CallParam(expr):
            dump_node(expr, level)
        case _:
            pass

def dump_array(arr, level):
    for x in arr:
        dump_node(x, level)

def dump_node(node, level = 0):
    prefix = "| " * level
    desc = short_desc(node)
    print(f"{prefix}|-{desc}")
    dump_children(node, level = level + 1)

