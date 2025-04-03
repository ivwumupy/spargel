from __future__ import annotations
from dataclasses import dataclass
import enum

class TokenKind(enum.Enum):
    UNKNOWN     = enum.auto()
    WHITESPACE  = enum.auto()
    IDENT       = enum.auto()
    OP          = enum.auto()

    L_PAREN     = enum.auto()
    R_PAREN     = enum.auto()
    L_BRACE     = enum.auto()
    R_BRACE     = enum.auto()

    SEMICOLON   = enum.auto()
    COLON       = enum.auto()
    COMMA       = enum.auto()
    PERIOD      = enum.auto()
    EQUAL       = enum.auto()

    INT_LIT     = enum.auto()

    KW_OPEN     = enum.auto()
    KW_FUNC     = enum.auto()

@dataclass
class Token:
    kind: TokenKind
    text: str

def dump_tokens(tokens: list[Token]):
    for tok in tokens:
        print(tok)
