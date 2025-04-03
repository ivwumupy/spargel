"""Concrete Syntax Tree
"""

import spargel.lexer as L
import spargel.abstract as A

def _recons_arr(arr):
    return ''.join([x.recons() for x in arr])

class Node:
    def dump(self, level = 0):
        prefix = "| " * level
        desc = self.short_desc()
        print(f"{prefix}|-{desc}")
        self.dump_children(level = level + 1)

class SourceFile(Node):
    """A source file.

    Fields:
        items: [ModuleItem] -- module items
        eof_tok: Token      -- token for end of file
    """
    def __init__(self, items, eof_tok):
        self.items = items
        self.eof_tok = eof_tok

    def short_desc(self):
        n = len(self.items)
        return f"SourceFile [{n} items]"

    def recons(self):
        return _recons_arr(self.items) + self.eof_tok.recons()

    def dump_children(self, level):
        for item in self.items:
            item.dump(level)

    def abstract(self):
        items = []
        for x in self.items:
            items.append(x.item.abstract())
        opens = [i.item.abstract() for i in self.items if isinstance(i.item, OpenDecl)]
        funcs = [i.item.abstract() for i in self.items if isinstance(i.item, FuncDecl)]
        return A.SourceFile(opens, funcs)

# Decl = OpenDecl | FuncDecl | ImplDecl
# TODO: Decl == ModuleItem?

class ModuleItem(Node):
    """An item in a module.

    Fields:
        item: OpenDecl | FuncDecl | ImplDecl
    """

    def __init__(self, item):
        self.item = item

    def short_desc(self):
        return "ModuleItem"

    def recons(self):
        return self.item.recons()

    def dump_children(self, level):
        self.item.dump(level)

class OpenDecl(Node):
    """An open declaration.

    Fields:
        open_tok: Token
        path: Token
    """

    def __init__(self, open_tok, path):
        self.open_tok = open_tok
        self.path = path

    def short_desc(self):
        return f"OpenDecl <path = {self.path.text}>"

    def recons(self):
        return self.open_tok.recons() + self.path.recons()

    def dump_children(self, level):
        pass

    def abstract(self):
        return A.OpenDecl(self.path.text)

#class ModulePath:
#    """The path of a module.
#    """
#    segments: "[ModuleName]"
#
#class ModuleName:
#    token: "Token"

class FuncDecl(Node):
    """A function declaration.

    Fields:
        attr: Attr
        func_tok: Token
        name: Token
        func_sig: FuncSig
        body: Expr
    """

    def __init__(self, attr, func_tok, name, func_sig, body):
        self.attr = attr
        self.func_tok = func_tok
        self.name = name
        self.func_sig = func_sig
        self.body = body

    def short_desc(self):
        return f"FuncDecl <name = {self.name.text}>"

    def recons(self):
        result = ""
        if self.attr is not None:
            result += self.attr.recons()
        result += self.func_tok.recons() + self.name.recons() + self.func_sig.recons() + self.body.recons()
        return result

    def dump_children(self, level):
        if self.attr is not None:
            self.attr.dump(level)
        self.func_sig.dump(level)
        self.body.dump(level)

    def start_loc(self):
        return (self.func_tok.line, self.func_tok.column)

    def abstract(self):
        params = []
        for p in self.func_sig.params.params:
            params.append(p.abstract())
        return A.FuncDecl(self.name.text, params, self.func_sig.ret.abstract(), self.body.abstract())

class FuncSig(Node):
    """Function signature

    Fields:
        lparen_tok: Token
        params: FuncParams
        rparen_tok: Token
    """

    def __init__(self, lparen_tok, params, rparen_tok, colon_tok, ret):
        self.lparen_tok = lparen_tok
        self.params = params
        self.rparen_tok = rparen_tok
        self.colon_tok = colon_tok
        self.ret = ret

    def short_desc(self):
        return "FuncSig"

    def recons(self):
        return self.lparen_tok.recons() + self.params.recons() + self.rparen_tok.recons()

    def dump_children(self, level):
        self.params.dump(level)

class FuncParams(Node):
    """Function parameters

    Fields:
        params: [FuncParam]
    """

    def __init__(self, params):
        self.params = params

    def short_desc(self):
        n = len(self.params)
        return f"FuncParams [{n} params]"

    def recons(self):
        return _recons_arr(self.params)

    def dump_children(self, level):
        for p in self.params:
            p.dump(level)

class FuncParam(Node):
    """Function parameter

    Fields:
        attr: Attr
        name: Token
        colon_tok: Token?
        type: Expr?
        comma_tok: Token?
    """

    def __init__(self, attr, name, colon_tok, type, comma_tok):
        self.attr = attr
        self.name = name
        self.colon_tok = colon_tok
        self.type = type
        self.comma_tok = comma_tok

    def short_desc(self):
        return f"FuncParam <name = {self.name.text}>"

    def recons(self):
        result = ""

        if self.attr is not None:
            result += self.attr.recons()
        result += self.name.recons()
        if self.colon_tok is not None:
            result += self.colon_tok.recons()
        if self.type is not None:
            result += self.type.recons()
        if self.comma_tok is not None:
            result += self.comma_tok.recons()
        return result

    def dump_children(self, level):
        if self.attr is not None:
            self.attr.dump(level)
        if self.type is not None:
            self.type.dump(level)

    def abstract(self):
        return A.FuncParam(self.name.text, self.type.abstract())

#######
# Expr
#

class GroupedExpr(Node):
    """A parenthesized expression

    Fields:
        lparen_tok: Token
        expr: Expr
        rparen_tok: Token
    """

    def __init__(self, lparen_tok, expr, rparen_tok):
        self.lparen_tok = lparen_tok
        self.expr = expr
        self.rparen_tok = rparen_tok

    def short_desc(self):
        return "GroupedExpr"

    def recons(self):
        return self.lparen_tok.recons() + self.expr.recons() + self.rparen_tok.recons()

    def dump_children(self, level):
        self.expr.dump(level)

class IdentExpr(Node):
    """Identifier expression

    Fields:
        token: Token
    """

    def __init__(self, token):
        self.token = token

    def short_desc(self):
        return f"IdentExpr <name = {self.token.text}>"

    def recons(self):
        return self.token.recons()

    def dump_children(self, level):
        pass

    def abstract(self):
        return A.IdentExpr(self.token.text)

class LitExpr(Node):
    """Literal expression

    Fields:
        token: Token
    """

    def __init__(self, token):
        self.token = token

    def short_desc(self):
        return f"LitExpr <value = {self.token.text}>"

    def recons(self):
        return self.token.recons()

    def dump_children(self, level):
        pass

    def abstract(self):
        return A.LitExpr(self.token.text)

class AddExpr(Node):
    """
    Fields:
        left: Expr
        add_tok: Token
        right: Expr
    """

    def __init__(self, left, add_tok, right):
        self.left = left
        self.add_tok = add_tok
        self.right = right

    def short_desc(self):
        return "AddExpr"

    def recons(self):
        return self.left.recons() + self.add_tok.recons() + self.right.recons()

    def dump_children(self, level):
        self.left.dump(level)
        self.right.dump(level)

    def abstract(self):
        return A.CallExpr(A.IdentExpr("add"), [self.left.abstract(), self.right.abstract()])

class DivExpr(Node):
    """
    Fields:
        left: Expr
        div_tok: Token
        right: Expr
    """

    def __init__(self, left, div_tok, right):
        self.left = left
        self.div_tok = div_tok
        self.right = right

    def short_desc(self):
        return "DivExpr"

    def recons(self):
        return self.left.recons() + self.div_tok.recons() + self.right.recons()

    def dump_children(self, level):
        self.left.dump(level)
        self.right.dump(level)

class CallExpr(Node):
    """
    Fields:
        func: "Expr"
        lparen_tok: "Token"
        params: "CallParams"
        rparen_tok: "Token"
    """

    def __init__(self, func, lparen_tok, params, rparen_tok):
        self.func = func
        self.lparen_tok = lparen_tok
        self.params = params
        self.rparen_tok = rparen_tok

    def short_desc(self):
        return "CallExpr"

    def recons(self):
        return self.func.recons() + self.lparen_tok.recons() + self.params.recons() + self.rparen_tok.recons()

    def dump_children(self, level):
        self.func.dump(level)
        self.params.dump(level)

    def abstract(self):
        params = [p.expr.abstract() for p in self.params.params]
        return A.CallExpr(self.func.abstract(), params)

class CallParams(Node):
    """
    Fields:
        params: "[CallParam]"
    """

    def __init__(self, params):
        self.params = params

    def short_desc(self):
        n = len(self.params)
        return f"CallParams [{n} params]"

    def recons(self):
        return _recons_arr(self.params)

    def dump_children(self, level):
        for p in self.params:
            p.dump(level)

class CallParam(Node):
    """
    Fields:
        expr: "Expr"
        comma_tok: "Token?"
    """

    def __init__(self, expr, comma_tok):
        self.expr = expr
        self.comma_tok = comma_tok

    def short_desc(self):
        return "CallParam"

    def recons(self):
        result = self.expr.recons()
        if self.comma_tok is not None:
            result += self.comma_tok.recons()
        return result

    def dump_children(self, level):
        self.expr.dump(level)

class BlockExpr(Node):
    """
    lbrace_tok: "Token"
    items: "[BlockItem]"
    rbrace_tok: "Token"
    """

    def __init__(self, lbrace_tok, items, rbrace_tok):
        self.lbrace_tok = lbrace_tok
        self.items = items
        self.rbrace_tok = rbrace_tok

    def short_desc(self):
        n = len(self.items)
        return f"BlockExpr [{n} items]"

    def recons(self):
        return self.lbrace_tok.recons() + _recons_arr(self.items) + self.rbrace_tok.recons()

    def dump_children(self, level):
        for i in self.items:
            i.dump(level)

    def abstract(self):
        items = [i.item.abstract() for i in self.items]
        return A.BlockExpr(items)

class TypeExpr(Node):
    """
    Fields:
        expr: Expr
    """

    def __init__(self, expr):
        self.expr = expr

    def short_desc(self):
        return "TypeExpr"

    def recons(self):
        return self.expr.recons()

    def dump_children(self, level):
        self.expr.dump(level)

    def abstract(self):
        return self.expr.abstract()

# Stmt = ExprStmt | LetStmt | RetStmt

class BlockItem(Node):
    """
    item: "ExprStmt | LetStmt | RetStmt"
    """

    def __init__(self, item):
        self.item = item

    def short_desc(self):
        return "BlockItem"

    def recons(self):
        return self.item.recons()

    def dump_children(self, level):
        self.item.dump(level)

class ExprStmt(Node):
    """
    expr: "Expr"
    semicolon_tok: "Token"
    """

    def __init__(self, expr, semicolon_tok):
        self.expr = expr
        self.semicolon_tok = semicolon_tok

    def short_desc(self):
        return "ExprStmt"

    def recons(self):
        return self.expr.recons() + self.semicolon_tok.recons()

    def dump_children(self, level):
        self.expr.dump(level)

    def abstract(self):
        return self.expr.abstract()

class LetStmt(Node):
    """
    let_tok: "Token"
    name: "Token"
    equal_tok: "Token"
    expr: "Expr"
    semicolon_tok: "Token"
    """

    def __init__(self, let_tok, name, equal_tok, expr, semicolon_tok):
        self.let_tok = let_tok
        self.name = name
        self.equal_tok = equal_tok
        self.expr = expr
        self.semicolon_tok = semicolon_tok

    def short_desc(self):
        return f"LetStmt <name = {self.name.text}>"

    def recons(self):
        return self.let_tok.recons() + self.name.recons() + self.equal_tok.recons() + self.expr.recons() + self.semicolon_tok.recons()

    def dump_children(self, level):
        self.expr.dump(level)

    def abstract(self):
        return A.LetStmt(self.name.text, None, self.expr.abstract())

class RetStmt(Node):
    """
    return_tok: "Token"
    expr: "Expr"
    semicolon_tok: "Token"
    """

    def __init__(self, return_tok, expr, semicolon_tok):
        self.return_tok = return_tok
        self.expr = expr
        self.semicolon_tok = semicolon_tok

    def short_desc(self):
        return "RetStmt"

    def recons(self):
        return self.return_tok.recons() + self.expr.recons() + self.semicolon_tok.recons()

    def dump_children(self, level):
        self.expr.dump(level)

    def abstract(self):
        return A.RetStmt(self.expr.abstract())

class Attr(Node):
    """
    Fields:
        at_tok: Token
        expr: Expr
    """
    def __init__(self, at_tok, expr):
        self.at_tok = at_tok
        self.expr = expr

    def short_desc(self):
        return "Attr"

    def recons(self):
        return self.at_tok.recons() + self.expr.recons()

    def dump_children(self, level):
        self.expr.dump(level)
