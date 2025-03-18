// Lexer

const fs = require("node:fs");
const { Buffer } = require("node:buffer");

const D = require("./diagnostic.js");
const S = require("./source.js");

function Lexer(source, diag) {
    this.source = source;
    this.buffer = source.content;
    this.position = 0;
    this.line_info = [];
    // index
    this.current_line = 0;
    this.diag = diag;
}

Lexer.prototype.scanLines = function () {
    let idx = -1;
    do {
        // At the beginning of the loop, `idx` points to the last "\n".
        // Now move to the start of the next line.
        idx = idx + 1;
        this.line_info.push({ start: idx, });
        // Search for the next "\n".
        idx = this.buffer.indexOf(0x0a, idx); // "\n"
    } while (idx !== -1);
};

Lexer.prototype.lineStart = function (line) {
    if (line < this.line_info.length) {
        return this.line_info[line].start;
    }
    return this.buffer.length;
};

Lexer.prototype.makeLocation = function (line, start, end) {
    let line_start = this.lineStart(line);
    let line_end = this.lineStart(line + 1);
    return new D.DiagnosticLocation(
        this.source.filename,
        line + 1, // line_number
        this.buffer.subarray(line_start, line_end - 1),
        start - line_start,
        end - line_start
    );
};

function showChar(n) {
    if (n >= 0x20 && n <= 0x7e) {
        return String.fromCharCode(n);
    }
    return "0x" + n.toString(16);
}

Lexer.prototype.lex = function () {
    while (this.position < this.buffer.length) {
        let c = this.buffer[this.position];

        this.diag.emit(
            D.Diagnostic.error(
                this.makeLocation(this.current_line, this.position, this.position + 1),
                D.DiagnosticId.InvalidCharacter,
                [showChar(c)]
            )
        );

        switch (c) {
            case 0x0a:
                // this.handleLF();
                this.current_line += 1;
                break;

            case 0x20:
                // this.handleSpace();
                break;

            default:
                // this.handleUnknown();
                break;
        }

        this.position += 1;
    }
};

Lexer.prototype.handleUnknown = function () {

};

Lexer.prototype.handleWhitespace = function () {
};

Lexer.prototype.handleNewline = function () {
};

// source: S.SourceBuffer
// tokens: Array(Token)
// line_info: Array(LineInfo)
function TokenBuffer(source, tokens, line_info) {
    this.source = source;
    this.tokens = tokens;
    this.line_info = line_info;
}

// diag: D.DiagnosticEngine
TokenBuffer.lexSource = (source, diag) => {
    diag = diag || new D.DiagnosticEngine();

    let lexer = new Lexer(source, diag);

    // First find the start of each line.
    lexer.scanLines();

    lexer.lex();

    return new TokenBuffer(source, [], lexer.line_info);
};

TokenBuffer.lexString = (s, diag) => {
    return TokenBuffer.lexSource(S.SourceBuffer.fromString(s), diag);
};

module.exports = {
    TokenBuffer,
};

// TokenBuffer.lexString(`
// import std
// 
// def main()
//     print("hello, world")
// `);
