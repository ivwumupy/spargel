import { Buffer } from "node:buffer";
import { readFileSync } from "node:fs";
import process from "node:process";
import { parseArgs, styleText } from "node:util";

import { buildEnum } from "./utils.js";
import { lexSource } from "./lexer.js";
import { parseTokens, dumpSyntaxNode } from "./parser.js";

// content: Buffer
class SourceBuffer {
    constructor(name, content) {
        this.name = name;
        this.content = content;
    }
    static fromPath(path) {
        const data = readFileSync(path);
        return new SourceBuffer(path, data);
    }
    static fromString(name, s) {
        return new SourceBuffer(name, Buffer.from(s));
    }
}

function main() {
    const { values, positionals } = parseArgs({
        options: {
            "dump-ast": { type: "boolean" },
        },
        allowPositionals: true,
    });

    if (positionals.length === 0) {
        console.log("error: no input file");
        return 1;
    }

    const file = positionals[0];

    const source = SourceBuffer.fromPath(file);
    const tokens = lexSource(source.content);
    const node = parseTokens(tokens);

    dumpSyntaxNode(node);

    return 0;
}

process.exit(main());
