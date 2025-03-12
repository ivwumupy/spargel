#include <spargel/base/check.h>
#include <spargel/base/test.h>
#include <spargel/lang/syntax.h>

using spargel::lang::Lexer;
using spargel::lang::TokenKind;

TEST(Lexer_Basic) {
    Lexer l("def hello()\n  print(\"hello, world\")\n");
    spargel_check(!l.isEnd());
    spargel_check(l.peekByte() == 'd');
    l.advance();
    spargel_check(l.peekByte() == 'e');
    l.advance();
    spargel_check(l.peekByte() == 'f');

    l.reset();

    l.eatWhile([](char c) { return c >= 'a' && c <= 'z'; });
    spargel_check(l.peekByte() == ' ');
}

TEST(Lexer_Newline) {
    Lexer l("\n\r\n\rxyz\n");
    auto x = l.lexNewline();
    spargel_check(!l.isEnd() && l.peekByte() == 'x');
    spargel_check(x.isLeft());
    spargel_check(x.left().kind == TokenKind::newline);
    spargel_check(x.left().length() == 4);
}
