const { suite, test } = require("node:test");
const assert = require("node:assert/strict");

const { Buffer } = require("node:buffer");

const L = require("./lexer.js");

suite("TokenBuffer", () => {
    test("line_info", () => {
        let b;

        b = L.TokenBuffer.lexString(`import oo
            def main() {}
        `);
        assert.equal(b.line_info.length, 3);

        b = L.TokenBuffer.lexString(``);
        assert.equal(b.line_info.length, 1);

        b = L.TokenBuffer.lexString(`
        `);
        assert.equal(b.line_info.length, 2);
    });
});
