import dataclasses as DC
import enum

class TokenKind(enum.Enum):
    UNKNOWN         = enum.auto()
    END_OF_FILE     = enum.auto()

    NEWLINE         = enum.auto()
    WHITESPACE      = enum.auto()

    LINE_COMMENT    = enum.auto()

    IDENTIFIER      = enum.auto()

    INTEGER_LITERAL = enum.auto()
    FLOAT_LITERAL   = enum.auto()

    LEFT_PAREN      = enum.auto()
    RIGHT_PAREN     = enum.auto()
    LEFT_BRACE      = enum.auto()
    RIGHT_BRACE     = enum.auto()

    EQUAL           = enum.auto()
    SEMICOLON       = enum.auto()
    COMMA           = enum.auto()
    COLON           = enum.auto()
    PERIOD          = enum.auto()
    AT              = enum.auto()

class KeywordKind(enum.Enum):
    OPEN        = enum.auto()
    FUNC        = enum.auto()
    IMPL        = enum.auto()
    LOOP        = enum.auto()
    BREAK       = enum.auto()
    CONTINUE    = enum.auto()
    RETURN      = enum.auto()
    LET         = enum.auto()

@DC.dataclass
class Token:
    kind: "TokenKind"
    start: "int"
    end: "int"
    text: "str"

    line: "int" = -1
    column: "int" = -1

    leading_trivia: "[Token]" = DC.field(default_factory = list)
    trailing_trivia: "[Token]" = DC.field(default_factory = list)

    keyword_candidate: "KeywordKind?" = None

def is_trivia_token(tok):
    return tok.kind in {
        TokenKind.NEWLINE,
        TokenKind.WHITESPACE,
        TokenKind.LINE_COMMENT,
    }

def is_dec_digit(c):
    return c >= "0" and c <="9"

def is_bin_digit(c):
    return c == "0" or c == "1"

def is_hex_digit(c):
    return (c >= "0" and c <= "9") or (c >= "a" and c <= "f")

IDENT_START_CHARS = {
    *"abcdefghijklmnopqrstuvwxyz",
    *"ABCDEFGHIJKLMNOPQRSTUVWXYZ",
    *"~!$%^&*-+|<>?/_",
}

IDENT_CONT_CHARS = {
    *"abcdefghijklmnopqrstuvwxyz",
    *"ABCDEFGHIJKLMNOPQRSTUVWXYZ",
    *"~!$%^&*-+|<>?/_",
    *"1234567890",
}

def is_ident_begin(c):
    return c in IDENT_START_CHARS

def is_ident_cont(c):
    return c in IDENT_CONT_CHARS

class Cursor:
    def __init__(self, source):
        """
        Args
            source: bytes
        """
        self.source = source
        self.position = 0
        self.start = 0
        self.tokens = []

    def is_end(self):
        return self.position >= len(self.source)
    def advance(self):
        if self.is_end():
            raise Exception("advancing a cursor that has reached the end")
        self.position += 1
    def advance_while(self, cond):
        while not self.is_end():
            c = self.peek()
            if not cond(c):
                break
            self.advance()
    def peek(self):
        if self.position >= len(self.source):
            return None
        return chr(self.source[self.position])
    def peek_next(self):
        if self.position + 1 >= len(self.source):
            return None
        return chr(self.source[self.position + 1])

    def emit(self, kind):
        self.tokens.append(Token(
            kind, self.start, self.position,
            self.source[self.start:self.position].decode("utf-8")))
        
    def lex(self):
        while not self.is_end():
            c = self.peek()
            self.start = self.position
            match c:
                case "\n":
                    self.handle_newline()
                case " ":
                    self.handle_whitespace()
                case "(":
                    self.handle_lparen()
                case ")":
                    self.handle_rparen()
                case "{":
                    self.handle_lbrace()
                case "}":
                    self.handle_rbrace()
                case "=":
                    self.handle_equal()
                case ",":
                    self.handle_comma()
                case ";":
                    self.handle_semicolon()
                case ":":
                    self.handle_colon()
                case "/":
                    self.handle_slash()
                case c if is_dec_digit(c):
                    self.handle_number()
                case c if is_ident_begin(c):
                    self.handle_identifier()
                case _:
                    self.handle_unknown()
        self.emit(TokenKind.END_OF_FILE)

    def handle_newline(self):
        self.advance()
        self.emit(TokenKind.NEWLINE)

    def handle_unknown(self):
        self.advance()
        self.emit(TokenKind.UNKNOWN)

    def handle_whitespace(self):
        self.advance_while(lambda c: c == " ")
        self.emit(TokenKind.WHITESPACE)

    def handle_lparen(self):
        self.advance()
        self.emit(TokenKind.LEFT_PAREN)

    def handle_rparen(self):
        self.advance()
        self.emit(TokenKind.RIGHT_PAREN)

    def handle_lbrace(self):
        self.advance()
        self.emit(TokenKind.LEFT_BRACE)

    def handle_rbrace(self):
        self.advance()
        self.emit(TokenKind.RIGHT_BRACE)

    def handle_equal(self):
        self.advance()
        self.emit(TokenKind.EQUAL)

    def handle_comma(self):
        self.advance()
        self.emit(TokenKind.COMMA)

    def handle_semicolon(self):
        self.advance()
        self.emit(TokenKind.SEMICOLON)

    def handle_colon(self):
        self.advance()
        self.emit(TokenKind.COLON)

    def handle_slash(self):
        c = self.peek_next()
        match c:
            case "/":
                self.handle_line_comment()
            case _:
                self.handle_identifier()

    def handle_line_comment(self):
        self.advance_while(lambda c: c != "\n")
        self.emit(TokenKind.LINE_COMMENT)

    def handle_number(self):
        c = self.peek()
        match c:
            case "0":
                b = self.peek_next()
                match b:
                    case "b":
                        self.handle_bin_number()
                    case "x":
                        self.handle_hex_number()
                    case _:
                        self.handle_dec_number()
            case _:
                self.handle_dec_number()

    def handle_bin_number(self):
        # "0b"
        self.advance()
        self.advance()
        self.advance_while(lambda c: is_bin_digit(c))
        self.emit(TokenKind.INTEGER_LITERAL)

    def handle_hex_number(self):
        # "0x"
        self.advance()
        self.advance()
        self.advance_while(lambda c: is_hex_digit(c))
        self.emit(TokenKind.INTEGER_LITERAL)

    def handle_dec_number(self):
        self.advance_while(lambda c: is_dec_digit(c))
        c = self.peek()
        match c:
            case ".":
                self.advance()
                self.advance_while(lambda c: is_dec_digit(c))
                d = self.peek()
                match c:
                    case "e":
                        self.eat_exponent()
                    case _:
                        pass
                self.emit(TokenKind.FLOAT_LITERAL)
            case "e":
                self.advance()
                self.eat_exponent()
                self.emit(TokenKind.FLOAT_LITERAL)
            case _:
                self.emit(TokenKind.INTEGER_LITERAL)
    
    def eat_exponent(self):
        c = self.peek()
        if c == "-" or c == "+":
            self.advance()
        self.advance_while(lambda c: is_dec_digit(c))

    def handle_identifier(self):
        # self.advance_while(lambda c: is_ident_cont(c))
        while self.position < len(self.source):
            c = self.peek()
            if c == "/":
                d = self.peek_next()
                if d == "/":
                    self.emit(TokenKind.IDENTIFIER)
                    return
            if not is_ident_cont(c):
                break
            self.advance()
        self.emit(TokenKind.IDENTIFIER)

def collect_trailing_trivia(toks, start):
    pos = start + 1
    trailing = []
    while pos < len(toks):
        tok = toks[pos]
        if tok.kind == TokenKind.NEWLINE or not is_trivia_token(tok):
            break
        trailing.append(tok)
        pos += 1
    return trailing, pos


def fold_trivia(toks):
    pos = 0
    leading = []
    result = []
    while pos < len(toks):
        tok = toks[pos]
        if is_trivia_token(tok):
            leading.append(tok)
            pos += 1
        else:
            trailing, new_pos = collect_trailing_trivia(toks, pos)
            tok.leading_trivia = leading
            tok.trailing_trivia = trailing
            result.append(tok)
            pos = new_pos
            leading = []
    return result

def _set_keyword(tok, kind):
    tok.keyword_candidate = kind

def scan_keywords(toks):
    for tok in toks:
        if tok.kind == TokenKind.IDENTIFIER:
            match tok.text:
                case "open":
                    _set_keyword(tok, KeywordKind.OPEN)
                case "func":
                    _set_keyword(tok, KeywordKind.FUNC)
                case "let":
                    _set_keyword(tok, KeywordKind.LET)
                case "return":
                    _set_keyword(tok, KeywordKind.RETURN)
                case _:
                    pass

def lex_source(s):
    cursor = Cursor(s)
    cursor.lex()
    tokens = cursor.tokens
    tokens = fold_trivia(tokens)
    scan_keywords(tokens)
    return tokens
