import argparse
import pathlib
import sys
# from pprint import pp

from spargel.lexer import lex_source
from spargel.parser import parse_tokens
import spargel.concrete as C

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("file", type=pathlib.Path)
    args = parser.parse_args()
    
    source = args.file.read_bytes()
    tokens = lex_source(source)
    node = parse_tokens(tokens)
    C.dump_node(node)
    return 0

if __name__ == '__main__':
    sys.exit(main())
