import argparse
import pathlib
import sys

from spargel.lexer import lex_source
from spargel.parser import parse_tokens
import spargel.concrete as C
import spargel.abstract as A

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("file", type=pathlib.Path)
    args = parser.parse_args()
    
    source = args.file.read_bytes()
    tokens = lex_source(source)
    node = parse_tokens(tokens)
    node.dump()
    #print("==== Reconstruct ====")
    #print(node.recons())
    anode = node.abstract()
    #print("==== AST ====")
    #print(anode)
    anode.resolve_names(mods = [A.std_mod])
    anode.tyck()
    print("==== CPP ====")
    print(anode.cpp())
    return 0

if __name__ == '__main__':
    sys.exit(main())
