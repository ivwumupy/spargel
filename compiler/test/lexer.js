import test from "node:test";
import assert from "node:assert/strict";

import { lexSource, TokenKind } from "../src/lexer.js"

function lexString(s, opts) {
    return lexSource(Buffer.from(s), opts);
}

function checkTokenKinds(tokens, expected) {
    assert.equal(tokens.length, expected.length);
    for (let i = 0; i < tokens.length; i++) {
        assert.equal(tokens[i].kind, expected[i]);
    }
}

test("empty", (t) => {
    checkTokenKinds(
        lexString("", { fold_trivia: false }),
        [TokenKind.EndOfFile]
    );
    checkTokenKinds(
        lexString("", { fold_trivia: true }),
        [TokenKind.EndOfFile]
    );
});

test("whitespace", (t) => {
    const opts = {
        fold_trivia: false,
    };

    checkTokenKinds(
        lexString(" ", opts),
        [TokenKind.Whitespace, TokenKind.EndOfFile]
    );
    checkTokenKinds(
        lexString("  ", opts),
        [TokenKind.Whitespace, TokenKind.EndOfFile]
    );
    checkTokenKinds(
        lexString(" a ", opts),
        [TokenKind.Whitespace, TokenKind.Identifier, TokenKind.Whitespace, TokenKind.EndOfFile]
    );

    opts.fold_trivia = true;

    checkTokenKinds(
        lexString(" ", opts),
        [TokenKind.EndOfFile]
    );
    checkTokenKinds(
        lexString("  ", opts),
        [TokenKind.EndOfFile]
    );
    checkTokenKinds(
        lexString(" a ", opts),
        [TokenKind.Identifier, TokenKind.EndOfFile]
    );

});

test("newline", (t) => {
    const opts = {
        fold_trivia: false,
    };

    checkTokenKinds(
        lexString("\n", opts),
        [TokenKind.Newline, TokenKind.EndOfFile]
    );
    checkTokenKinds(
        lexString("\n ", opts),
        [TokenKind.Newline, TokenKind.Whitespace, TokenKind.EndOfFile]
    );
    checkTokenKinds(
        lexString("\n\n", opts),
        [TokenKind.Newline, TokenKind.Newline, TokenKind.EndOfFile]
    );
    checkTokenKinds(
        lexString("\n \n", opts),
        [TokenKind.Newline, TokenKind.Whitespace, TokenKind.Newline, TokenKind.EndOfFile]
    );
    checkTokenKinds(
        lexString("a\nbn", opts),
        [TokenKind.Identifier, TokenKind.Newline, TokenKind.Identifier, TokenKind.EndOfFile]
    );
});

test("paren", (t) => {
    checkTokenKinds(
        lexString("("),
        [TokenKind.LeftParen, TokenKind.EndOfFile]
    );
    checkTokenKinds(
        lexString(")"),
        [TokenKind.RightParen, TokenKind.EndOfFile]
    );
    checkTokenKinds(
        lexString("()"),
        [TokenKind.LeftParen, TokenKind.RightParen, TokenKind.EndOfFile]
    );
    checkTokenKinds(
        lexString("(xyz)"),
        [TokenKind.LeftParen, TokenKind.Identifier, TokenKind.RightParen, TokenKind.EndOfFile]
    );
});

test("symbols", (t) => {
    checkTokenKinds(
        lexString("(=.,;@}{)"),
        [
            TokenKind.LeftParen,
            TokenKind.Equal,
            TokenKind.Period,
            TokenKind.Comma,
            TokenKind.Semicolon,
            TokenKind.At,
            TokenKind.RightBrace,
            TokenKind.LeftBrace,
            TokenKind.RightParen,
            TokenKind.EndOfFile,
        ]
    );
});

test("trivia", (t) => {
    let tokens = lexString("  a  \nb=  ");
    checkTokenKinds(tokens, [
        TokenKind.Identifier,
        TokenKind.Identifier,
        TokenKind.Equal,
        TokenKind.EndOfFile,
    ]);
    checkTokenKinds(tokens[0].leading_trivia, [
        TokenKind.Whitespace,
    ]);
    checkTokenKinds(tokens[0].trailing_trivia, [
        TokenKind.Whitespace,
    ]);
    checkTokenKinds(tokens[1].leading_trivia, [
        TokenKind.Newline,
    ]);
    checkTokenKinds(tokens[1].trailing_trivia, []);
    checkTokenKinds(tokens[2].leading_trivia, []);
    checkTokenKinds(tokens[2].trailing_trivia, [
        TokenKind.Whitespace,
    ]);
    checkTokenKinds(tokens[3].leading_trivia, []);
    checkTokenKinds(tokens[3].trailing_trivia, []);
});
