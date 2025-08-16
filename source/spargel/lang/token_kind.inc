#ifndef TOKEN_KIND
#define TOKEN_KIND(name)
#endif

#ifndef KEYWORD_TOKEN_KIND
#define KEYWORD_TOKEN_KIND(name, text) TOKEN_KIND(name)
#endif

TOKEN_KIND(identifier)

TOKEN_KIND(float_literal)
TOKEN_KIND(integer_literal)
TOKEN_KIND(string_literal)

KEYWORD_TOKEN_KIND(kw_func, func)
KEYWORD_TOKEN_KIND(kw_if, if)
KEYWORD_TOKEN_KIND(kw_loop, loop)
KEYWORD_TOKEN_KIND(kw_open, open)
KEYWORD_TOKEN_KIND(kw_return, return)

TOKEN_KIND(at_sign)
TOKEN_KIND(colon)
TOKEN_KIND(comma)
TOKEN_KIND(period)

TOKEN_KIND(left_brace)
TOKEN_KIND(right_brace)
TOKEN_KIND(left_paren)
TOKEN_KIND(right_paren)
TOKEN_KIND(left_square)
TOKEN_KIND(right_square)

TOKEN_KIND(line_comment)

TOKEN_KIND(whitespace)
TOKEN_KIND(end_of_file)
TOKEN_KIND(unknown)

#undef TOKEN_KIND
#undef KEYWORD_TOKEN_KIND
