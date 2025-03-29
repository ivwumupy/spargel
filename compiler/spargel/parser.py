"""Parser

SourceFile
    = ModuleItem* <END_OF_FILE>

ModuleItem
    = FuncDecl
    | OpenDecl

FuncDecl
    = <func> <IDENT> FuncSig Expr

FuncSig
    = <(> FuncParams <)> FuncRet?

FuncParams
    = FuncParam*

FuncParam
    = <IDENT> (<:> Expr)? <,>?

FuncRet
    = <:> Expr

Expr
    = BlockExpr
    | CallExpr
    | LitExpr
    | IdentExpr

LitExpr
    = <LITERAL>

IdentExpr
    = <IDENT>

CallExpr
    = Expr <(> CallParams <)>

CallParams
    = CallParam*

CallParam
    = Expr <,>?

BlockExpr
    = <{> BlockItem <}>

BlockItem
    = Expr
    | Stmt

Stmt
    = LetStmt
    | ExprStmt
"""

from spargel.lexer import Token, TokenKind, KeywordKind
import spargel.concrete as C

from pprint import pp

def is_module_item_start(tok):
    if tok.kind != TokenKind.IDENTIFIER:
        return False
    return tok.keyword_candidate in {
        KeywordKind.OPEN,
        KeywordKind.FUNC,
    }

def is_func_param_start(tok):
    return tok.kind == TokenKind.IDENTIFIER

class Parser:
    def __init__(self, tokens):
        self.tokens = tokens
        self.position = 0
    def is_end(self):
        return self.position >= len(self.tokens)
    def advance(self):
        if self.is_end():
            raise Exception("advancing a cursor that has reached the end")
        self.position += 1
    def peek(self):
        if self.position >= len(self.tokens):
            return None
        return self.tokens[self.position]
    def peek_next(self):
        if self.position + 1 >= len(self.tokens):
            return None
        return self.tokens[self.position + 1]

    def eat(self, kind):
        tok = self.peek()
        if tok is None or tok.kind != kind:
            return Token(kind, -1, -1, "")
        self.advance()
        return tok

    def parse_source_file(self):
        """
        -> C.SourceFile
        """
        items = []
        eof_tok = None
        while not self.is_end():
            tok = self.peek()
            if not is_module_item_start(tok):
                break
            item = self.parse_module_item()
            items.append(item)
        eof_tok = self.eat(TokenKind.END_OF_FILE)
        return C.SourceFile(items, eof_tok)

    def parse_module_item(self):
        """
        -> C.ModuleItem
        """
        tok = self.peek()
        match tok.keyword_candidate:
            case KeywordKind.OPEN:
                item = self.parse_open_decl()
                return C.ModuleItem(item)
            case KeywordKind.FUNC:
                item = self.parse_func_decl()
                return C.ModuleItem(item)
            case _:
                raise Exception("internal error")

    def parse_open_decl(self):
        """
        -> C.OpenDecl
        """
        open_tok = self.eat(TokenKind.IDENTIFIER)
        path = self.eat(TokenKind.IDENTIFIER)
        return C.OpenDecl(open_tok, path)

    def parse_func_decl(self):
        """
        -> C.FuncDecl
        """
        func_tok = self.eat(TokenKind.IDENTIFIER)
        name = self.eat(TokenKind.IDENTIFIER)
        func_sig = self.parse_func_sig()
        body = self.parse_expr()
        return C.FuncDecl(func_tok, name, func_sig, body)

    def parse_func_sig(self):
        """
        -> C.FuncSig
        """
        lparen_tok = self.eat(TokenKind.LEFT_PAREN)
        params = self.parse_func_params()
        rparen_tok = self.eat(TokenKind.RIGHT_PAREN)
        return C.FuncSig(lparen_tok, params, rparen_tok)

    def parse_func_params(self):
        """
        -> C.FuncParams
        """
        params = []
        while not self.is_end():
            tok = self.peek()
            if not is_func_param_start(tok):
                break
            param = self.parse_func_param()
            params.append(param)
        return C.FuncParams(params)

    def parse_func_param(self):
        """
        -> C.FuncParam
        """
        name = self.eat(TokenKind.IDENTIFIER)
        colon_tok = None
        type = None
        comma_tok = None

        tok = self.peek()
        if tok.kind == TokenKind.COLON:
            colon_tok = tok
            self.advance()
            type = self.parse_expr()

        tok = self.peek()
        if tok.kind == TokenKind.COMMA:
            comma_tok = tok
            self.advance()

        return C.FuncParam(name, colon_tok, type, comma_tok)

    def parse_expr(self, min_lbp = 0):
        left = self.parse_expr_nud()
        while True:
            lbp = self.parse_expr_lbp()
            if lbp < min_lbp:
                break

            tok = self.peek()
            tmp = self.parse_expr_led(left)

            # guard
            if tmp is None:
                raise Exception("internal error")

            left = tmp
        return left

    # a fused nud
    def parse_expr_nud(self):
        tok = self.peek()
        match tok.kind:
            case TokenKind.IDENTIFIER:
                self.advance()
                return C.IdentExpr(tok)
            case TokenKind.INTEGER_LITERAL:
                self.advance()
                return C.LitExpr(tok)
            case TokenKind.FLOAT_LITERAL:
                self.advance()
                return C.LitExpr(tok)
            case TokenKind.LEFT_PAREN:
                lparen_tok = tok
                self.advance()
                expr = self.parse_expr()
                rparen_tok = self.eat(TokenKind.RIGHT_PAREN)
                return C.GroupedExpr(lparen_tok, expr, rparen_tok)
            case TokenKind.LEFT_BRACE:
                return self.parse_block_expr()
            case _:
                print(f"nud: {tok}")
                raise Exception("internal error")

    # a fused lbp
    def parse_expr_lbp(self):
        tok = self.peek()
        match tok.kind:
            case TokenKind.IDENTIFIER if tok.text == "+":
                return 1
            case TokenKind.LEFT_PAREN:
                return 3
            case _:
                return -1

    # a fused led
    def parse_expr_led(self, left):
        tok = self.peek()
        match tok.kind:
            # the `+` operator
            case TokenKind.IDENTIFIER if tok.text == "+":
                add_tok = tok
                self.advance()
                right = self.parse_expr(min_lbp = 2)
                return C.AddExpr(left, add_tok, right)

            # `(` in the non-prefix position; must be a func call
            case TokenKind.LEFT_PAREN:
                lparen_tok = tok
                self.advance()
                params = self.parse_call_params()
                rparen_tok = self.eat(TokenKind.RIGHT_PAREN)
                return C.CallExpr(left, lparen_tok, params, rparen_tok)

            # the following are end of expr
            case TokenKind.COMMA:
                return None
            case TokenKind.RIGHT_PAREN:
                return None

            case _:
                print(f"nud: {tok}")
                raise Exception("internal error")

    def parse_call_params(self):
        params = []
        while not self.is_end():
            tok = self.peek()
            if tok.kind == TokenKind.RIGHT_PAREN:
                break
            param = self.parse_call_param()
            params.append(param)
        return C.CallParams(params)

    def parse_call_param(self):
        expr = self.parse_expr()
        comma_tok = None
        tok = self.peek()
        if tok.kind == TokenKind.COMMA:
            comma_tok = tok
            self.advance()
        return C.CallParam(expr, comma_tok)

    def parse_block_expr(self):
        lbrace_tok = self.eat(TokenKind.LEFT_BRACE)
        items = []
        while not self.is_end():
            tok = self.peek()
            if tok.kind == TokenKind.RIGHT_BRACE:
                break
            item = self.parse_block_item()
            items.append(item)
        rbrace_tok = self.eat(TokenKind.RIGHT_BRACE)
        return C.BlockExpr(lbrace_tok, items, rbrace_tok)

    def parse_block_item(self):
        tok = self.peek()
        match tok.kind:
            case TokenKind.IDENTIFIER if tok.keyword_candidate == KeywordKind.LET:
                item = self.parse_let_stmt()
                return C.BlockItem(item)
            case TokenKind.IDENTIFIER if tok.keyword_candidate == KeywordKind.RETURN:
                item = self.parse_ret_stmt()
                return C.BlockItem(item)
            case _:
                item = self.parse_expr_stmt()
                return C.BlockItem(item)

    def parse_let_stmt(self):
        let_tok = self.eat(TokenKind.IDENTIFIER)
        name = self.eat(TokenKind.IDENTIFIER)
        equal_tok = self.eat(TokenKind.EQUAL)
        expr = self.parse_expr()
        semicolon_tok = self.eat(TokenKind.SEMICOLON)
        return C.LetStmt(let_tok, name, equal_tok, expr, semicolon_tok)

    def parse_ret_stmt(self):
        return_tok = self.eat(TokenKind.IDENTIFIER)
        expr = self.parse_expr()
        semicolon_tok = self.eat(TokenKind.SEMICOLON)
        return C.RetStmt(return_tok, expr, semicolon_tok)

    def parse_expr_stmt(self):
        expr = self.parse_expr()
        semicolon_tok = self.eat(TokenKind.SEMICOLON)
        return C.ExprStmt(expr, semicolon_tok)

def parse_tokens(tokens):
    parser = Parser(tokens)
    return parser.parse_source_file()
