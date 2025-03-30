"""Concrete Syntax Tree
"""

from dataclasses import dataclass

import spargel.lexer as L

#@dataclass
class SourceFile:
    """A source file.

    Fields:
        items: [ModuleItem] -- module items
        eof_tok: Token      -- token for end of file
    """
    #items: "[ModuleItem]"
    #eof_tok: "Token"

    def __init__(self, items, eof_tok):
        self.items = items
        self.eof_tok = eof_tok

    def short_desc(self):
        n = len(self.items)
        return f"SourceFile [{n} items]"

# Decl = OpenDecl | FuncDecl | ImplDecl
# TODO: Decl == ModuleItem?

@dataclass
class ModuleItem:
    """An item in a module.
    """
    item: "OpenDecl | FuncDecl | ImplDecl"

    def short_desc(self):
        return "ModuleItem"

@dataclass
class OpenDecl:
    """An open declaration.
    """
    open_tok: "Token"
    path: "Token"
    #path: "ModulePath"

    def short_desc(self):
        return f"OpenDecl <path = {self.path.text}>"

#@dataclass
#class ModulePath:
#    """The path of a module.
#    """
#    segments: "[ModuleName]"
#
#@dataclass
#class ModuleName:
#    token: "Token"

@dataclass
class FuncDecl:
    """A function declaration.
    """
    func_tok: "Token"
    name: "Token"
    func_sig: "FuncSig"
    body: "Expr"

    def short_desc(self):
        return f"FuncDecl <name = {self.name.text}>"

@dataclass
class FuncSig:
    """Function signature
    """
    lparen_tok: "Token"
    params: "FuncParams"
    rparen_tok: "Token"

    def short_desc(self):
        return "FuncSig"

@dataclass
class FuncParams:
    """Function parameters
    """
    params: "[FuncParam]"

    def short_desc(self):
        n = len(self.params)
        return f"FuncParams [{n} params]"

@dataclass
class FuncParam:
    """Function parameter
    """
    name: "Token"
    colon_tok: "Token?"
    type: "Expr?"
    comma_tok: "Token?"

    def short_desc(self):
        return f"FuncParam <name = {self.name.text}>"

@dataclass
class ImplDecl:
    impl_tok: "Token"
    name: "Token"
    lbrace_tok: "Token"
    funcs: "[FuncDecl]"
    rbrace_tok: "Token"

    def short_desc(self):
        return "ImplDecl"

#######
# Expr
#

@dataclass
class GroupedExpr:
    """A parenthesized expression
    """
    lparen_tok: "Token"
    expr: "Expr"
    rparen_tok: "Token"

    def short_desc(self):
        return "GroupedExpr"

@dataclass
class IdentExpr:
    """Identifier expression
    """
    token: "Token"

    def short_desc(self):
        return f"IdentExpr <name = {self.token.text}>"

@dataclass
class LitExpr:
    """Literal expression
    """
    token: "Token"

    def short_desc(self):
        return f"LitExpr <value = {self.token.text}>"

@dataclass
class AddExpr:
    left: "Expr"
    add_tok: "Token"
    right: "Expr"

    def short_desc(self):
        return "AddExpr"

@dataclass
class CallExpr:
    func: "Expr"
    lparen_tok: "Token"
    params: "CallParams"
    rparen_tok: "Token"

    def short_desc(self):
        return "CallExpr"

@dataclass
class CallParams:
    params: "[CallParam]"

    def short_desc(self):
        n = len(self.params)
        return f"CallParams [{n} params]"

@dataclass
class CallParam:
    expr: "Expr"
    comma_tok: "Token?"

    def short_desc(self):
        return "CallParam"

@dataclass
class BlockExpr:
    lbrace_tok: "Token"
    items: "[BlockItem]"
    rbrace_tok: "Token"

    def short_desc(self):
        n = len(self.items)
        return f"BlockExpr [{n} items]"

# Stmt = ExprStmt | LetStmt | RetStmt

@dataclass
class BlockItem:
    item: "ExprStmt | LetStmt | RetStmt"

    def short_desc(self):
        return "BlockItem"

@dataclass
class ExprStmt:
    expr: "Expr"
    semicolon_tok: "Token"

    def short_desc(self):
        return "ExprStmt"

@dataclass
class LetStmt:
    let_tok: "Token"
    name: "Token"
    equal_tok: "Token"
    expr: "Expr"
    semicolon_tok: "Token"

    def short_desc(self):
        return f"LetStmt <name = {self.name.text}"

@dataclass
class RetStmt:
    return_tok: "Token"
    expr: "Expr"
    semicolon_tok: "Token"

    def short_desc(self):
        return "RetStmt"

def short_desc(node):
    return node.short_desc()
    #match node:
    #    case SourceFile(items, eof_tok):
    #        n = len(items)
    #        return f"SourceFile [{n} items]"
    #    case ModuleItem(item):
    #        return "ModuleItem"
    #    case OpenDecl(open_tok, path):
    #        return f"OpenDecl <path = {path.text}>"
    #    case FuncDecl(func_tok, name, func_sig, body):
    #        return f"FuncDecl <name = {name.text}>"
    #    case FuncSig(lparen_tok, params, rparen_tok):
    #        # TODO
    #        return "FuncSig [() -> Unit]"
    #    case FuncParams(params):
    #        n = len(params)
    #        return f"FuncParams [{n} params]"
    #    case FuncParam(name, colon_tok, type):
    #        return f"FuncParam <name = {name.text}>"
    #    case AddExpr(left, add_tok, right):
    #        return f"AddExpr"
    #    case IdentExpr(tok):
    #        return f"IdentExpr <name = {tok.text}>"
    #    case LitExpr(tok):
    #        return f"Literal <value = {tok.text}>"
    #    case GroupedExpr(lparen_tok, expr, rparen_tok):
    #        return "GroupedExpr"
    #    case CallExpr(func, lparen_tok, params, rparen_tok):
    #        return "CallExpr"
    #    case CallParams(params):
    #        return "CallParams"
    #    case CallParam(expr, comma_tok):
    #        return "CallParam"
    #    case BlockExpr(lbrace_tok, items, rbrace_tok):
    #        return "BlockExpr"
    #    case BlockItem(item):
    #        return "BlockItem"
    #    case ExprStmt(expr, semicolon_tok):
    #        return "ExprStmt"
    #    case LetStmt(let_tok, name, equal_tok, expr, semicolon_tok):
    #        return f"LetStmt <name = {name.text}>"
    #    case RetStmt(return_tok, expr, semicolon_tok):
    #        return "RetStmt"
    #    case _:
    #        return "<Unknown>"

def dump_children(node, level):
    match node:
        case SourceFile(items, eof):
            dump_array(items, level)
        case ModuleItem(item):
            dump_node(item, level)
        case FuncDecl(func_tok, name, func_sig, body):
            dump_node(func_sig, level)
            dump_node(body, level)
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
        case CallParam(expr, comma_tok):
            dump_node(expr, level)
        case BlockExpr(lbrace_tok, items, rbrace_tok):
            dump_array(items, level)
        case BlockItem(item):
            dump_node(item, level)
        case ExprStmt(expr, semicolon_tok):
            dump_node(expr, level)
        case LetStmt(let_tok, name, equal_tok, expr, semicolon_tok):
            dump_node(expr, level)
        case RetStmt(return_tok, expr, semicolon_tok):
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

def _recons_tokarr(arr):
    result = ""
    for t in arr:
        result += _recons_token(t)
    return result

def _recons_token(tok):
    if tok is None:
        return ""
    result = (_recons_tokarr(tok.leading_trivia) + 
              tok.text + 
              _recons_tokarr(tok.trailing_trivia))
    return result

def reconstruct_array(arr):
    result = ""
    for n in arr:
        result += reconstruct_source(n)
    return result

def reconstruct_source(node):
    if node is None:
        return ""
    match node:
        case SourceFile(items, eof_token):
            return reconstruct_array(items)
        case ModuleItem(item):
            return reconstruct_source(item)
        case OpenDecl(open_tok, path):
            return _recons_token(open_tok) + _recons_token(path)
        case FuncDecl(func_tok, name, func_sig, body):
            return (
                _recons_token(func_tok) +
                _recons_token(name) +
                reconstruct_source(func_sig) +
                reconstruct_source(body)
            )
        case FuncSig(lparen_tok, params, rparen_tok):
            return (
                _recons_token(lparen_tok) +
                reconstruct_source(params) +
                _recons_token(rparen_tok)
            )
        case FuncParams(params):
            return reconstruct_array(params)
        case FuncParam(name, colon_tok, type, comma_tok):
            return (
                _recons_token(name) +
                _recons_token(colon_tok) +
                reconstruct_source(type) +
                _recons_token(comma_tok)
            )
        case GroupedExpr(lparen_tok, expr, rparen_tok):
            return (
                _recons_token(lparen_tok) +
                reconstruct_source(expr) +
                _recons_token(rparen_tok)
            )
        case IdentExpr(tok):
            return _recons_token(tok)
        case LitExpr(tok):
            return _recons_token(tok)
        case AddExpr(left, add_tok, right):
            return (
                reconstruct_source(left) +
                _recons_token(add_tok) +
                reconstruct_source(right)
            )
        case CallExpr(func, lparen_tok, params, rparen_tok):
            return (
                reconstruct_source(func) +
                _recons_token(lparen_tok) +
                reconstruct_source(params) +
                _recons_token(rparen_tok)
            )
        case CallParams(params):
            return reconstruct_array(params)
        case CallParam(expr, comma_tok):
            return (
                reconstruct_source(expr) +
                _recons_token(comma_tok)
            )
        case BlockExpr(lbrace_tok, items, rbrace_tok):
            result = ""
            result += _recons_token(lbrace_tok)
            result += reconstruct_array(items)
            result += _recons_token(rbrace_tok)
            return result
        case BlockItem(item):
            return reconstruct_source(item)
        case ExprStmt(expr, semicolon_tok):
            return (
                reconstruct_source(expr) +
                _recons_token(semicolon_tok)
            )
        case LetStmt(let_tok, name, equal_tok, expr, semicolon_tok):
            return (
                _recons_token(let_tok) +
                _recons_token(name) +
                _recons_token(equal_tok) +
                reconstruct_source(expr) +
                _recons_token(semicolon_tok)
            )
        case RetStmt(return_tok, expr, semicolon_tok):
            return (
                _recons_token(return_tok) +
                reconstruct_source(expr) +
                _recons_token(semicolon_tok)
            )
        case _:
            return "?"


