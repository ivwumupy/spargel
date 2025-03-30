class SourceFile {
    items; // [ModuleItem]
    eof_tok; // Token

    constructor(items, eof_tok) {
        this.items = items
        this.eof_tok = eof_tok
    }

    short_desc() {
        const n = this.items.length;
        return `SourceFile [${n} items]`;
    }
}
