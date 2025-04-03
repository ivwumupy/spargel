from __future__ import annotations

from spargel.parse.token import Token, TokenKind
import spargel.syntax.concrete as C

class Parser:
    def __init__(self, tokens: list[Token]):
        self.tokens = tokens
        self.position = 0
    def peek(self) -> Token | None:
        if self.position >= len(self.tokens):
            return None
        return self.tokens[self.position]
    def advance(self):
        if self.position < len(self.tokens):
            self.position += 1
    def eat(self, kind: TokenKind) -> Token:
        if (tok := self.peek()) and tok.kind == kind:
            self.advance()
            return tok
        return Token(kind, text="")
    def parse_SourceFile(self) -> C.SourceFile:
        decls: list[C.Decl] = []
        while tok := self.peek():
            if tok.kind == TokenKind.KW_OPEN:
                decl = self.parse_OpenDecl()
                decls.append(decl)
            else:
                self.advance()
        return C.SourceFile(decls)
    def parse_OpenDecl(self) -> C.OpenDecl:
        open = self.eat(TokenKind.KW_OPEN)
        module = self.parse_ModuleName()
        return C.OpenDecl(open, module)
    def parse_ModuleName(self) -> C.ModuleName:
        name = self.eat(TokenKind.IDENT)
        return C.ModuleName(name)

def parse_tokens(tokens: list[Token]) -> C.SourceFile:
    parser = Parser(tokens)
    return parser.parse_SourceFile()
