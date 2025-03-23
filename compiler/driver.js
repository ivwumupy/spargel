"use strict";

const assert = require("node:assert/strict");
const { Buffer } = require("node:buffer");
const fs = require("node:fs");
const process = require("node:process");

// content: Buffer
function SourceBuffer(name, content) {
    assert(content instanceof Buffer);

    this.name = name;
    this.content = content;
}

SourceBuffer.fromPath = path => {
    const data = fs.readFileSync(path);
    return new SourceBuffer(path, data);
};

SourceBuffer.fromString = (name, s) => {
    return new SourceBuffer(name, Buffer.from(s));
};

// line -- 1-based
// column -- the byte offset from the start of the line; 1-based
// position -- the byte offset in the source buffer; 0-based
function SourceLoc(line, column, position, file) {
    this.line = line;
    this.column = column;
    this.position = position;
    this.file = file;
}

function SourceSpan(start, end) {
    this.start = start;
    this.end = end;
}

const TokenKind = {
    At: 'at', // '@'
    Comma: 'comma', // ','
    Equal: 'equal', // '='
    Exclamation: 'exclamation', // '!'
    Identifier: 'identifier',
    LeftBrace: 'left_brace', // '{'
    LeftParen: 'left_paren', // '('
    LineComment: 'line_comment',
    Newline: 'newline', // '\n'
    Period: 'period', // '.'
    RightBrace: 'right_brace', // '}'
    RightParen: 'right_paren', // ')'
    Semicolon: 'semicolon', // ';'
    Unknown: 'unknown',
    Whitespace: 'whitespace', // ' ' (one or more)
};

// Trivia are attached to tokens.
//   Reason: It's hard to decide which ast node should own the trivia.
//
// trivia: [Token]
//
function Token(kind, location, text, leading_trivia, trailing_trivia, diagnostic) {
    assert(location instanceof SourceSpan);

    this.kind = kind;
    this.location = location;
    this.text = text;
    this.leading_trivia = leading_trivia;
    this.trailing_trivia = trailing_trivia;
    this.diagnostic = diagnostic;
}

// source: SourceBuffer
function Lexer(source) {
    assert(source instanceof SourceBuffer);

    this.source = source;
    this.content = this.source.content;
    this.length = this.content.length;
    this.position = 0;
    // this.line = 0;
    this.tokens = [];
    // this.line_start = [];
}

Lexer.prototype.peek = function () {
    if (this.position >= this.length) return -1;
    return this.content[this.position];
}

Lexer.prototype.advance = function () {
    if (this.position < this.length) this.position += 1;
}

Lexer.prototype.advanceWhile = function (f) {
    while (this.position < this.length) {
        let c = this.peek();
        if (!f(c)) break;
        this.advance();
    }
}

function isNewline(c) {
    return c == 0x0a; // '\n'
}

function isWhitespace(c) {
    return c == 0x20; // ' '
}

function isDoubleQuote(c) {
    return c == 0x22; // '"'
}

function isIdentifierBegin(c) {
    return (c >= 0x61 && c <= 0x7a) // 'a'-'z'
        || (c >= 0x41 && c <= 0x5a) // 'A'-'Z'
        || (c == 0x5f) // '_'
        ;
}

function isIdentifierContinuation(c) {
    return (c >= 0x61 && c <= 0x7a) // 'a'-'z'
        || (c >= 0x41 && c <= 0x5a) // 'A'-'Z'
        || (c == 0x5f) // '_'
        || (c >= 0x30 && c <= 0x39) // '0'-'9'
        ;
}

function isLeftParen(c) {
    return c == 0x28; // '('
}

function isRightParen(c) {
    return c == 0x29; // ')'
}

function isLeftBrace(c) {
    return c == 0x7b; // '{'
}

function isRightBrace(c) {
    return c == 0x7d; // '}'
}

function isSemicolon(c) {
    return c == 0x3b; // ';'
}

// Lexer.prototype.scanLines = function () {
//     let idx = -1;
//     do {
//         // At the beginning of the loop, `idx` points to the last '\n'.
//         // Now move to the start of the next line.
//         idx = idx + 1;
//         this.line_start.push(idx);
//         // Search for the next '\n'.
//         idx = this.content.indexOf(0x0a, idx); // '\n'
//     } while (idx !== -1);
// };

// Lexer.prototype.currentLoc = function () {
//     let line_offset = this.line_start[this.line];
//     let column = this.position - line_offset;
//     return new SourceLoc(this.line + 1, column + 1, this.position, this.source.name);
// };

// start, end -- byte offset from the beginning of the buffer
Lexer.prototype.emitToken = function (kind, start, end) {
    this.tokens.push({
        kind: kind,
        start: start,
        end: end,
        source_present: true,
    });
};

// The lexer has multiple passes:
//   // 0. Compute line beginnings.
//   1. Recognize all tokens.
//   2. Annotate the tokens with source locations and indentation.
//   3. Insert braces and semicolons;
//   4. Fold trivia.
Lexer.prototype.lex = function () {
    // this.scanLines();
    this.scanTokens();
    this.annotateTokens();
    this.foldTrivia();
    this.decodeText();
    this.scanKeywords();
};

Lexer.prototype.scanTokens = function () {
    while (this.position < this.length) {
        // c: Int
        const c = this.peek();

        if (isNewline(c)) { this.handleNewline(); continue; }
        if (isWhitespace(c)) { this.handleWhitespace(); continue; }

        if (isLeftParen(c)) { this.handleLeftParen(); continue; }
        if (isRightParen(c)) { this.handleRightParen(); continue; }
        if (isLeftBrace(c)) { this.handleLeftBrace(); continue; }
        if (isRightBrace(c)) { this.handleRightBrace(); continue; }
        if (isSemicolon(c)) { this.handleSemicolon(); continue; }

        if (isIdentifierBegin(c)) { this.handleIdentifier(); continue; }

        this.handleUnknown();
    }
};

Lexer.prototype.handleNewline = function () {
    let start = this.position;

    this.advance();
    this.line += 1;

    let end = this.position;

    this.emitToken(TokenKind.Newline, start, end);
};

Lexer.prototype.handleWhitespace = function () {
    let start = this.position;

    this.advanceWhile(c => {
        return isWhitespace(c);
    });

    let end = this.position;

    this.emitToken(TokenKind.Whitespace, start, end);
};

Lexer.prototype.handleIdentifier = function () {
    let start = this.position;

    this.advanceWhile(c => {
        return isIdentifierContinuation(c);
    });

    let end = this.position;

    this.emitToken(TokenKind.Identifier, start, end);
};

Lexer.prototype.handleLeftParen = function () {
    let start = this.position;
    this.advance();
    let end = this.position;
    this.emitToken(TokenKind.LeftParen, start, end);
};

Lexer.prototype.handleRightParen = function () {
    let start = this.position;
    this.advance();
    let end = this.position;
    this.emitToken(TokenKind.RightParen, start, end);
};

Lexer.prototype.handleLeftBrace = function () {
    let start = this.position;
    this.advance();
    let end = this.position;
    this.emitToken(TokenKind.LeftBrace, start, end);
};

Lexer.prototype.handleRightBrace = function () {
    let start = this.position;
    this.advance();
    let end = this.position;
    this.emitToken(TokenKind.RightBrace, start, end);
};

Lexer.prototype.handleSemicolon = function () {
    let start = this.position;
    this.advance();
    let end = this.position;
    this.emitToken(TokenKind.Semicolon, start, end);
};

Lexer.prototype.handleUnknown = function () {
    let start = this.position;
    this.advance();
    let end = this.position;
    this.emitToken(TokenKind.Unknown, start, end);
};

function isTriviaToken(tok) {
    return tok.kind === TokenKind.Newline
        || tok.kind === TokenKind.Whitespace
        || tok.kind === TokenKind.LineComment
        ;
}

// We are not doing brace and semicolon insertions for MVP.
// So the syntax is not indentation sensitive.
Lexer.prototype.annotateTokens = function () {
    // 1-based
    let line = 1;
    let line_offset = 0;

    for (let tok of this.tokens) {
        tok.line = line;
        // 1-based
        tok.column = tok.start - line_offset + 1;

        if (tok.kind === TokenKind.Newline) {
            line += 1;
            // the next line
            line_offset = tok.end;
        }
    }
};

// A token owns all trailing trivia until a newline.
Lexer.prototype.foldTrivia = function () {
    let leading_trivia = [];
    let trailing_trivia = [];

    let result = [];

    let idx = 0;

    while (idx < this.tokens.length) {
        let tok = this.tokens[idx];

        if (isTriviaToken(tok)) {
            leading_trivia.push(tok);
            idx += 1;
        } else {
            // `tok` is non-trivia.
            // Eat all trivia until newline.
            idx += 1;
            while (idx < this.tokens.length) {
                let tok = this.tokens[idx];
                if (!isTriviaToken(tok) || tok.kind === TokenKind.Newline) break;
                trailing_trivia.push(tok);
                idx += 1;
            }
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
};

Lexer.prototype.decodeText = function () {
    for (let tok of this.tokens) {
        if (tok.kind === TokenKind.Identifier) {
            tok.text = this.content.toString("utf8", tok.start, tok.end);
        }
    }
};

const KeywordKind = {
    Func: "func",
    Open: "open",
    Impl: "impl",
    Trait: "trait",
};

Lexer.prototype.scanKeywords = function () {
    function set(tok, kind) {
        tok.keyword_like = true;
        tok.keyword_candidate = kind;
    }

    for (let tok of this.tokens) {
        if (tok.kind === TokenKind.Identifier) {
            if (tok.text === "func") { set(tok, KeywordKind.Func); continue; }
            if (tok.text === "open") { set(tok, KeywordKind.Open); continue; }
            if (tok.text === "impl") { set(tok, KeywordKind.Impl); continue; }
            if (tok.text === "trait") { set(tok, KeywordKind.Trait); continue; }
            tok.keyword_like = false;
        }
    }
};

// tokens: [Token]
function TokenBuffer(tokens) {
    this.tokens = tokens;
}

TokenBuffer.fromSource = source => {
    let lexer = new Lexer(source);
    lexer.lex();
    return new TokenBuffer(lexer.tokens);
};

TokenBuffer.fromString = (name, s) => {
    return TokenBuffer.fromSource(SourceBuffer.fromString(name, s));
};

const SyntaxNodeKind = {
    // Children:
    //   - token
    //
    // TODO: When the token is unexpected, the parser should eat until it finds a known recover token.
    Unexpected: 'unexpected',

    // It's not a good idea to make "Missing" a separate node.
    //
    // Children:
    //   - children
    // Missing: 'missing',

    SourceFile: 'source_file',

    // Function Declaration
    // 
    //   @shader.fragment
    //   func foo(x: float3)
    //      return x
    // 
    // Children:
    //  - attributes: [`@shader.fragment`]
    //  - func_keyword: `func`
    //  - name: `foo`
    //  - signature: `(x: float3)`
    //  - body
    //
    FuncDecl: 'func_decl',

    // Function Signature
    // 
    //   (x: float 3): float4
    //
    FuncSig: 'func_sig',

    Block: 'block',

    // Import Declaration
    //
    //   @attr
    //   open std as S;
    //
    // Children:
    //  - attributes: [`@attr`]
    //  - open_keyword: `open`
    //  - path: `std`
    //  - ???
    //
    OpenDecl: 'open_decl',

    ModuleName: 'module_name',

    // Attribute
    //
    //   @location(5)
    //
    // Children:
    //  - keyword: `@`
    //  - name: `location`
    //  - left_paren: `(`
    //  - arguments: `5`
    //  - righ_paren: `)`
    //
    Attribute: 'attribute',
};

function SyntaxNode(kind) {
    this.kind = kind;
}

function Parser(tokens) {
    this.tokens = tokens;
    this.position = 0;
    // a stack to store results
    this.nodes = [];
}

Parser.prototype.peek = function () {
    return this.tokens[this.position];
};

Parser.prototype.advance = function () {
    if (this.position >= this.tokens.length) return;
    this.position += 1;
};

Parser.prototype.parseSourceFile = function () {
    let children = [];

    while (this.position < this.tokens.length) {
        let tok = this.tokens[this.position];

        if (tok.kind === TokenKind.Identifier) { children.push(this.handleSourceFileIdentifier()); continue; }
        if (tok.kind === TokenKind.At) { this.handleAt(node); continue; }

        this.handleUnknown();
    }

    // console.dir(this.nodes, { depth: null });

    return {
        kind: SyntaxNodeKind.SourceFile,
        children,
    };
};

// If an identifier appears as a top-level token, then it has to a keyword.
Parser.prototype.handleSourceFileIdentifier = function () {
    let tok = this.peek();
    if (!tok.keyword_like) {
        this.nodes.push({
            kind: SyntaxNodeKind.Unexpected,
            token: tok,
        });
        this.advance();
        return this.nodes.pop();
    }
    if (tok.keyword_candidate === KeywordKind.Func) {
        return this.parseFuncDecl();
    }
    if (tok.keyword_candidate === KeywordKind.Open) {
        this.parseOpenDecl();
        return this.nodes.pop();
    }
    assert(false);
};

Parser.prototype.parseFuncDecl = function () {
    let func_keyword = this.peek();
    this.advance();

    let name = this.peek();
    if (!name || name.kind !== TokenKind.Identifier) {
        name = {
            kind: TokenKind.Identifier,
            source_present: false,
        };
    } else {
        this.advance();
    }

    let signature = this.parseFuncSig();

    let body = this.parseBlock();

    return {
        kind: SyntaxNodeKind.FuncDecl,
        func_keyword,
        name,
        signature,
        body,
    };
};

Parser.prototype.parseFuncSig = function () {
    let left_paren = this.peek();
    if (!left_paren || left_paren.kind !== TokenKind.LeftParen) {
        left_paren = {
            kind: TokenKind.LeftParen,
            source_present: false,
        };
    } else {
        this.advance();
    }

    let right_paren = this.peek();
    if (!right_paren || right_paren.kind !== TokenKind.RightParen) {
        right_paren = {
            kind: TokenKind.RightParen,
            source_present: false,
        };
    } else {
        this.advance();
    }

    return {
        kind: SyntaxNodeKind.FuncSig,
        left_paren,
        right_paren,
    };
};

Parser.prototype.parseBlock = function () {
    let left_brace = this.peek();
    if (!left_brace || left_brace.kind !== TokenKind.LeftBrace) {
        left_brace = {
            kind: TokenKind.LeftBrace,
            source_present: false,
        };
    } else {
        this.advance();
    }

    let right_brace = this.peek();
    if (!right_brace || right_brace.kind !== TokenKind.RightBrace) {
        right_brace = {
            kind: TokenKind.RightBrace,
            source_present: false,
        };
    } else {
        this.advance();
    }

    return {
        kind: SyntaxNodeKind.Block,
        left_brace,
        right_brace,
    };
};

Parser.prototype.parseOpenDecl = function () {
    let open_keyword = this.peek();
    this.advance();

    this.parseModuleName();
    let name = this.nodes.pop();

    // `;`
    // let tok = this.peek();
    // if (!tok || tok.kind !== TokenKind.Semicolon) {
    //     tok = {
    //         kind: TokenKind.Semicolon,
    //         source_present: false,
    //     };
    // } else {
    //     this.advance();
    // }

    this.nodes.push({
        kind: SyntaxNodeKind.OpenDecl,
        open_keyword,
        name,
        // semicolon: tok,
    });
};

Parser.prototype.parseModuleName = function () {
    let tok = this.peek();

    if (!tok || tok.kind !== TokenKind.Identifier) {
        tok = {
            kind: TokenKind.Identifier,
            source_present: false,
        };
    } else {
        // `tok` is an identifier.
        this.advance();
    }

    this.nodes.push({
        kind: SyntaxNodeKind.ModuleName,
        token: tok,
    });
};

Parser.prototype.handleAt = function (parent) {
    this.advance();
};

Parser.prototype.handleUnknown = function (parent) {
    this.advance();
};

SyntaxNode.fromString = s => {
    let tok_buf = TokenBuffer.fromString("<input>", s);
    let parser = new Parser(tok_buf.tokens);
    return parser.parseSourceFile();
};

console.dir(SyntaxNode.fromString(`
open std
func main() {
}
`), { depth: null });


// console.log(process.argv);
