const { suite, test } = require("node:test");
const assert = require("node:assert/strict");

const { Buffer } = require("node:buffer");

const S = require("./source.js");

suite("SourceBuffer", () => {
    test("fields", () => {
        let b = new S.SourceBuffer("abc", Buffer.from("def"));
        assert.equal(b.filename, "abc");
        assert(b.content instanceof Buffer);
    });
    test("length", () => {
        let b = S.SourceBuffer.fromString("hello");
        assert.equal(b.length(), 5);
    });
});

