import { buildEnum } from "./utils.js";

export const TokenKind = buildEnum([
    "Unknown",

    "EndOfFile",

    "Newline",
    "Whitespace",

    "LineComment",

    "Identifier",

    "LeftBrace",
    "RightBrace",
    "LeftParen",
    "RightParen",

    "At",
    "Comma",
    "Equal",
    "Period",
    "Semicolon",
]);

// Trivia are attached to tokens.
//   Reason: It's hard to decide which ast node should own the trivia.
//

function isNewline(c) {
    return c === 0x0a; // '\n'
}

function isWhitespace(c) {
    return c === 0x20; // ' '
}

function isDoubleQuote(c) {
    return c === 0x22; // '"'
}

function isIdentifierBegin(c) {
    return (
        (c >= 0x61 && c <= 0x7a) || // 'a'-'z'
        (c >= 0x41 && c <= 0x5a) || // 'A'-'Z'
        c === 0x5f // '_'
    );
}

function isIdentifierContinuation(c) {
    return (
        (c >= 0x61 && c <= 0x7a) || // 'a'-'z'
        (c >= 0x41 && c <= 0x5a) || // 'A'-'Z'
        c === 0x5f || // '_'
        (c >= 0x30 && c <= 0x39) // '0'-'9'
    );
}

function isLeftParen(c) {
    return c === 0x28; // '('
}

function isRightParen(c) {
    return c === 0x29; // ')'
}

function isLeftBrace(c) {
    return c === 0x7b; // '{'
}

function isRightBrace(c) {
    return c === 0x7d; // '}'
}

function isAtSign(c) {
    return c === 0x40; // '@'
}

function isComma(c) {
    return c === 0x2c; // ','
}

function isEqual(c) {
    return c === 0x3d; // '='
}

function isPeriod(c) {
    return c === 0x2e; // '.'
}

function isSemicolon(c) {
    return c === 0x3b; // ';'
}

function isTriviaToken(tok) {
    return (
        tok.kind === TokenKind.Newline ||
        tok.kind === TokenKind.Whitespace ||
        tok.kind === TokenKind.LineComment
    );
}

export const KeywordKind = buildEnum([
    "Func",
    "Open",
    "Impl",
    "Trait",
    "Return",
]);

class Lexer {
    // source: String
    constructor(source) {
        this.source = source;
        this.position = 0;
        this.tokens = [];
    }

    peek() {
        if (this.position >= this.source.length) return -1;
        return this.source[this.position];
    }

    advance() {
        if (this.position < this.source.length) this.position += 1;
    }

    advanceWhile(f) {
        while (this.position < this.source.length) {
            const c = this.peek();
            if (!f(c)) break;
            this.advance();
        }
    }

    // start, end -- byte offset from the beginning of the buffer
    emitToken(kind, start, end) {
        // console.log(`${kind}<${start}:${end}>`);
        this.tokens.push({
            kind: kind,
            start: start,
            end: end,
            source_present: true,
        });
    }

    // The lexer has multiple passes:
    //   1. Recognize all tokens.
    //   2. Annotate the tokens with source locations and indentation.
    //   3. Insert braces and semicolons;
    //   4. Fold trivia.
    lex(opts) {
        opts = opts || {
            fold_trivia: true,
        };
        // this.scanLines();
        this.scanTokens();
        this.annotateTokens();
        if (opts.fold_trivia)
            this.foldTrivia();
        this.decodeText();
        this.scanKeywords();
    }

    scanTokens() {
        const dispatch = [
            { cond: isNewline,          func: this.handleNewline, },
            { cond: isWhitespace,       func: this.handleWhitespace, },

            { cond: isLeftParen,        func: this.handleLeftParen, },
            { cond: isRightParen,       func: this.handleRightParen, },
            { cond: isLeftBrace,        func: this.handleLeftBrace, },
            { cond: isRightBrace,       func: this.handleRightBrace, },

            { cond: isAtSign,           func: this.handleAtSign, },
            { cond: isComma,            func: this.handleComma, },
            { cond: isEqual,            func: this.handleEqual, },
            { cond: isPeriod,           func: this.handlePeriod, },
            { cond: isSemicolon,        func: this.handleSemicolon, },

            { cond: isIdentifierBegin,  func: this.handleIdentifier, },

            { cond: x => true,          func: this.handleUnknown, },
        ];

        while (this.position < this.source.length) {
            // c: Int
            const c = this.peek();

            for (const d of dispatch) {
                if (d.cond(c)) {
                    d.func.call(this);
                    break;
                }
            }
        }

        this.emitToken(TokenKind.EndOfFile, this.position, this.position + 1);
    }

    // TODO: "\r\n"
    handleNewline() {
        const start = this.position;

        this.advance();

        const end = this.position;

        this.emitToken(TokenKind.Newline, start, end);
    }

    // TODO: Currently we only support ' '.
    handleWhitespace() {
        const start = this.position;

        this.advanceWhile((c) => {
            return isWhitespace(c);
        });

        const end = this.position;

        this.emitToken(TokenKind.Whitespace, start, end);
    }

    handleIdentifier() {
        const start = this.position;

        this.advanceWhile((c) => {
            return isIdentifierContinuation(c);
        });

        const end = this.position;

        this.emitToken(TokenKind.Identifier, start, end);
    }

    handleLeftParen() { this.handleOneChar(TokenKind.LeftParen); }

    handleRightParen() { this.handleOneChar(TokenKind.RightParen); }

    handleLeftBrace() { this.handleOneChar(TokenKind.LeftBrace); }

    handleRightBrace() { this.handleOneChar(TokenKind.RightBrace); }

    handleAtSign() { this.handleOneChar(TokenKind.At); }

    handleComma() { this.handleOneChar(TokenKind.Comma); }

    handleEqual() { this.handleOneChar(TokenKind.Equal); }

    handlePeriod() { this.handleOneChar(TokenKind.Period); }

    handleSemicolon() { this.handleOneChar(TokenKind.Semicolon); }

    handleUnknown() {
        const start = this.position;
        this.advance();
        const end = this.position;
        this.emitToken(TokenKind.Unknown, start, end);
    }

    // eat the current char and emit kind
    handleOneChar(kind) {
        const start = this.position;
        this.advance();
        const end = this.position;
        this.emitToken(kind, start, end);
    }

    // We are not doing brace and semicolon insertions for MVP.
    // So the syntax is not indentation sensitive.
    annotateTokens() {
        // 1-based
        let line = 1;
        let line_offset = 0;

        for (const tok of this.tokens) {
            tok.line = line;
            // 1-based
            tok.column = tok.start - line_offset + 1;

            if (tok.kind === TokenKind.Newline) {
                line += 1;
                // the next line
                line_offset = tok.end;
            }
        }
    }

    // (pos: Int, result: [Token]) -> (new_pos: Int)
    collectTrailingTrivia(pos, result) {
        let idx = pos + 1;
        while (idx < this.tokens.length) {
            const tok = this.tokens[idx];
            if (!isTriviaToken(tok) || tok.kind === TokenKind.Newline) {
                break;
            }
            result.push(tok);
            idx += 1;
        }
        return idx;
    }

    // A token owns all trailing trivia until a newline.
    foldTrivia() {
        let leading_trivia = [];
        let trailing_trivia = [];

        const result = [];

        let idx = 0;

        while (idx < this.tokens.length) {
            const tok = this.tokens[idx];

            if (isTriviaToken(tok)) {
                leading_trivia.push(tok);
                idx += 1;
            } else {
                // `tok` is non-trivia.
                // Eat all trivia until newline.
                idx = this.collectTrailingTrivia(idx, trailing_trivia);
                tok.leading_trivia = leading_trivia;
                tok.trailing_trivia = trailing_trivia;
                result.push({
                    ...tok,
                    leading_trivia,
                    trailing_trivia,
                });

                leading_trivia = [];
                trailing_trivia = [];
            }
        }

        this.tokens = result;
    }

    decodeText() {
        for (const tok of this.tokens) {
            if (tok.kind === TokenKind.Identifier) {
                tok.text = this.source.toString("utf8", tok.start, tok.end);
            }
        }
    }

    scanKeywords() {
        function set(tok, kind) {
            tok.keyword_like = true;
            tok.keyword_candidate = kind;
        }

        for (const tok of this.tokens) {
            if (tok.kind === TokenKind.Identifier) {
                if (tok.text === "func") {
                    set(tok, KeywordKind.Func);
                    continue;
                }
                if (tok.text === "open") {
                    set(tok, KeywordKind.Open);
                    continue;
                }
                if (tok.text === "impl") {
                    set(tok, KeywordKind.Impl);
                    continue;
                }
                if (tok.text === "trait") {
                    set(tok, KeywordKind.Trait);
                    continue;
                }
                if (tok.text === "return") {
                    set(tok, KeywordKind.Return);
                    continue;
                }
                tok.keyword_like = false;
            }
        }
    }
}

// (source: Buffer) -> (tokens: [Token])
export function lexSource(source, opts) {
    const lexer = new Lexer(source);
    lexer.lex(opts);
    return lexer.tokens;
}
