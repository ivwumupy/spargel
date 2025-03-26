import { TokenKind, KeywordKind } from "./lexer.js";
import { buildEnum } from "./utils.js";

const SyntaxNodeKind = buildEnum([
    // Children:
    //   - token: Token
    //
    // TODO: When the token is unexpected, the parser should eat until it finds a known recover token.
    // Unexpected: "unexpected",
    "Unexpected",

    // It's not a good idea to make "Missing" a separate node.
    //
    // Children:
    //   - children
    // Missing: 'missing',

    // Children:
    //   - children: [SyntaxNode]
    // SourceFile: "source_file",
    "SourceFile",

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
    // FuncDecl: "func_decl",
    "FuncDecl",

    // Function Signature
    //
    //   (x: float 3): float4
    //
    // FuncSig: "func_sig",
    "FuncSig",

    // Children:
    //   - children: [SyntaxNode]
    //
    // Block: "block",
    "Block",

    // Children:
    //   - item: SyntaxNode
    //
    // BlockItem: "block_item",

    // FuncCallExpr: "func_call_expr",
    "FuncCallExpr",

    // ReturnStmt: "return_stmt",
    "ReturnStmt",

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
    // OpenDecl: "open_decl",
    "OpenDecl",

    // ModuleName: "module_name",
    "ModuleName",

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
    // Attribute: "attribute",
    "Attribute",
]);

// function SyntaxNode(kind) {
//     this.kind = kind;
// }

class Parser {
    constructor(tokens) {
        this.tokens = tokens;
        this.position = 0;
        // a stack to store results
        this.nodes = [];
    }
    peek() {
        if (this.position >= this.tokens.length) return null;
        return this.tokens[this.position];
    }
    peekNext() {
        if (this.position + 1 >= this.tokens.length) return null;
        return this.tokens[this.position + 1];
    }
    advance() {
        if (this.position >= this.tokens.length) return;
        this.position += 1;
    }

    // Notation:
    //  `*` : >= 0
    //  `+` : >= 1
    //  `?` : optional

    // SourceFile
    //  = Item*
    //
    // Item
    //  = OpenDecl
    //  | FuncDecl
    //  | StructDecl
    //  | EnumDecl
    //  | TraitDecl
    //  | ImplDecl
    //
    // OpenDecl
    //  = `open` ModuleName
    //
    // ModuleName
    //  = Identifier
    //
    // FuncDecl
    //  = `func` Identifier FuncSignature Block
    //
    // FuncSignature
    //  = `(` FuncParams? `)` FuncRetType?
    //
    // FuncRetType
    //  = `->` Type
    //
    // FuncParams
    //  = FuncParam (`,` FuncParams)*
    //
    // Block
    //  = `{` Stmt* `}`
    //
    // Stmt
    //  = Item
    //  | LetStmt
    //  | ExprStmt
    //
    // LetStmt
    //  = `let` Identifier (`:` Type)? (`=` Expr)? `;`
    //
    // (TODO)
    // ExprStmt
    //  = Expr
    //
    // Expr
    //  = LitExpr
    // 
    parseSourceFile() {
        const children = [];

        while (this.position < this.tokens.length) {
            const tok = this.tokens[this.position];

            if (tok.kind === TokenKind.Identifier) {
                children.push(this.handleSourceFileIdentifier());
                continue;
            }
            if (tok.kind === TokenKind.At) {
                this.handleAt(node);
                continue;
            }

            this.handleUnknown();
        }

        // console.dir(this.nodes, { depth: null });
        return {
            kind: SyntaxNodeKind.SourceFile,
            children,
        };
    }
    // If an identifier appears as a top-level token, then it has to a keyword.
    handleSourceFileIdentifier() {
        const tok = this.peek();
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
        throw new Error();
    }

    // Consume and return a token with the given kind.
    // If the kind doesn't match, then construct a new token with `source_present = false`.
    eatToken(kind) {
        const tok = this.peek();
        if (!tok || tok.kind !== kind) {
            return {
                kind,
                source_present: false,
            };
        } 
        this.advance();
        return tok;
    }

    parseFuncDecl() {
        const func_keyword = this.peek();
        this.advance();

        const name = this.eatToken(TokenKind.Identifier);

        const signature = this.parseFuncSig();

        const body = this.parseBlock();

        return {
            kind: SyntaxNodeKind.FuncDecl,
            func_keyword,
            name,
            signature,
            body,
        };
    }
    parseFuncSig() {
        const left_paren = this.eatToken(TokenKind.LeftParen);
        const right_paren = this.eatToken(TokenKind.RightParen);

        return {
            kind: SyntaxNodeKind.FuncSig,
            left_paren,
            right_paren,
        };
    }
    parseBlock() {
        const left_brace = this.eatToken(TokenKind.LeftBrace);

        const children = [];

        while (this.position < this.tokens.length) {
            const tok = this.peek();
            if (tok.kind === TokenKind.RightBrace) {
                break;
            }
            children.push(this.parseBlockItem());
        }

        const right_brace = this.eatToken(TokenKind.RightBrace);

        return {
            kind: SyntaxNodeKind.Block,
            left_brace,
            children,
            right_brace,
        };
    }
    parseBlockItem() {
        const tok = this.peek();

        let item;

        if (tok.kind === TokenKind.Identifier) {
            item = this.parseBlockItemIdentifier();
        } else {
            this.advance();
            item = {
                kind: SyntaxNodeKind.Unexpected,
                token: tok,
            };
        }
        return item;
    }

    parseBlockItemIdentifier() {
        const ident = this.peek();

        if (ident.keyword_like && ident.keyword_candidate === KeywordKind.Return) { return this.parseReturnStmt(); }

        const next = this.peekNext();

        if (next && next.kind === TokenKind.LeftParen) {
            // `ident(`
            return this.parseFuncCallExpr();
        }

        this.advance();
        return {
            kind: SyntaxNodeKind.Unexpected,
            token: ident,
        };
    }

    parseFuncCallExpr() {
        const name = this.peek();
        this.advance();
        const left_paren = this.peek();
        this.advance();

        const right_paren = this.eatToken(TokenKind.RightParen);
        const semicolon = this.eatToken(TokenKind.Semicolon);

        return {
            kind: SyntaxNodeKind.FuncCallExpr,
            name,
            left_paren,
            right_paren,
            semicolon,
        };
    }

    parseOpenDecl() {
        const open_keyword = this.peek();
        this.advance();

        this.parseModuleName();
        const name = this.nodes.pop();

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
    }

    parseModuleName() {
        // let tok = this.peek();

        // if (!tok || tok.kind !== TokenKind.Identifier) {
        //     tok = {
        //         kind: TokenKind.Identifier,
        //         source_present: false,
        //     };
        // } else {
        //     // `tok` is an identifier.
        //     this.advance();
        // }
        
        const tok = this.eatToken(TokenKind.Identifier);

        this.nodes.push({
            kind: SyntaxNodeKind.ModuleName,
            token: tok,
        });
    }

    parseReturnStmt() {
        const return_keyword = this.eatToken(TokenKind.Identifier);
        const expr = this.parseExpr();
        const semicolon = this.eatToken(TokenKind.Semicolon);

        return {
            kind: SyntaxNodeKind.ReturnStmt,
            return_keyword,
            expr,
            semicolon,
        };
    }

    parseExpr() {
        const tok = this.peek();
    }

    handleAt(parent) {
        this.advance();
    }

    handleUnknown(parent) {
        this.advance();
    }
}

// function parseString(s) {
//     let tok_buf = TokenBuffer.fromString("<input>", s);
//     let parser = new Parser(tok_buf.tokens);
//     return parser.parseSourceFile();
// }
//
// source_file
// |-block_item
// | |-func_decl
// | | |- ...
// |-

export function parseTokens(tokens) {
    const parser = new Parser(tokens);
    return parser.parseSourceFile();
}

export function dumpSyntaxNode(node) {
    function showNode(node) {
        switch (node.kind) {
            case SyntaxNodeKind.OpenDecl:
                return node.name.token.text;
            case SyntaxNodeKind.FuncDecl:
                return node.name.text;
            case SyntaxNodeKind.Unexpected: {
                const tok = node.token;
                return `<line:${tok.line}, column: ${tok.column}>`;
            }
            default:
                return '';
        }
    }
    function dumpNode(node, prefix) {
        const node_desc = showNode(node);
        const node_str = `${prefix}|-${node.kind}: ${node_desc}`;
        console.log(node_str);

        const sub_prefix = `${prefix}| `;
        function dumpChild(n) {
            dumpNode(n, sub_prefix);
        }
        switch (node.kind) {
            case SyntaxNodeKind.SourceFile:
                node.children.forEach(dumpChild);
                break;
            case SyntaxNodeKind.OpenDecl:
                break;
            case SyntaxNodeKind.FuncDecl:
                dumpChild(node.body);
                break;
            case SyntaxNodeKind.Block:
                node.children.forEach(dumpChild);
                break;
            case SyntaxNodeKind.BlockItem:
                dumpChild(node.item);
                break;
        }
    }
    dumpNode(node, "");
}

// console.dir(
//     parseString(`
// open std
// func main() {
//     foo();
//     baz();
// }
// `),
//     { depth: null },
// );

