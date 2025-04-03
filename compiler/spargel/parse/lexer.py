from __future__ import annotations
from typing import Callable

from spargel.parse.token import Token, TokenKind

def is_whitespace(c: str):
    return c in {' ', '\n'}

def is_identifier_start(c: str):
    return c in {
            *"abcdefghijklmnopqrstuvwxyz",
            *"ABCDEFGHIJKLMNOPQRSTUVWXYZ",
            *"_",
        }

def is_identifier_cont(c: str):
    return c in {
            *"abcdefghijklmnopqrstuvwxyz",
            *"ABCDEFGHIJKLMNOPQRSTUVWXYZ",
            *"_",
            *"0123456789",
        }

def is_special_symbol(c: str):
    return c in {
            *"(){}:;,.="
        }

SPECIAL_SYMBOL_KIND = {
    "(": TokenKind.L_PAREN,
    ")": TokenKind.R_PAREN,
    "{": TokenKind.L_BRACE,
    "}": TokenKind.R_BRACE,
    ":": TokenKind.COLON,
    ";": TokenKind.SEMICOLON,
    ",": TokenKind.COMMA,
    ".": TokenKind.PERIOD,
    "=": TokenKind.EQUAL,
}

def is_number_start(c: str):
    return c in {*"0123456789"}

def is_dec_digit(c: str):
    return c in {*"0123456789"}

def check_keyword(s: str) -> TokenKind | None:
    match s:
        case "open":
            return TokenKind.KW_OPEN
        case "func":
            return TokenKind.KW_FUNC
        case _:
            return None

class Lexer:
    def __init__(self, source: str):
        self.source = source
        self.position = 0
        self.start = 0
        self.tokens = []
    def peek(self) -> str | None:
        if self.position >= len(self.source):
            return None
        return self.source[self.position]
    def advance(self):
        if self.position < len(self.source):
            self.position += 1
    def advance_while(self, f: Callable[[str], bool]):
        while c := self.peek():
            if not f(c):
                break
            self.advance()
    def mark_start(self):
        self.start = self.position
    def emit(self, kind: TokenKind):
        self.tokens.append(Token(kind, self.source[self.start:self.position]))
    def lex(self):
        while c := self.peek():
            self.mark_start()
            if is_whitespace(c):
                self.lex_whitespace()
            elif is_identifier_start(c):
                self.lex_identifier()
            elif is_special_symbol(c):
                self.lex_special_symbol(c)
            elif is_number_start(c):
                self.lex_number()
            else:
                self.advance()
                self.emit(TokenKind.UNKNOWN)
    def lex_whitespace(self):
        self.advance_while(is_whitespace)
        self.emit(TokenKind.WHITESPACE)
    def lex_identifier(self):
        self.advance_while(is_identifier_cont)
        if keyword := check_keyword(self.source[self.start:self.position]):
            self.emit(keyword)
        else:
            self.emit(TokenKind.IDENT)
    def lex_special_symbol(self, c: str):
        self.advance()
        self.emit(SPECIAL_SYMBOL_KIND[c])
    def lex_number(self):
        self.advance_while(is_dec_digit)
        self.emit(TokenKind.INT_LIT)

def lex_source(source: str) -> list[Token]:
    lexer = Lexer(source)
    lexer.lex()
    return lexer.tokens
