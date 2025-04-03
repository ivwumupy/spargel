import argparse
import pathlib
import sys

from spargel.parse.token import dump_tokens
from spargel.parse.lexer import lex_source
from spargel.parse.parser import parse_tokens
#import spargel.syntax.abstract as A
import spargel.syntax.concrete as C

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("file", type=pathlib.Path)
    parser.add_argument("--dump-tokens", action="store_true")
    parser.add_argument("--dump-concrete", action="store_true")
    args = parser.parse_args()

    source = args.file.read_text()
    tokens = lex_source(source)

    if args.dump_tokens:
        dump_tokens(tokens)

    croot = parse_tokens(tokens)

    if args.dump_concrete:
        C.dump_node(croot)

    return 0

if __name__ == '__main__':
    sys.exit(main())
