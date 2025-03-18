const { suite, test, beforeEach } = require("node:test");
const assert = require("node:assert/strict");

const D = require("./diagnostic.js");

suite("DiagnosticLevel", () => {
    const L = D.DiagnosticLevel;
    test("show", () => {
        assert.equal(L.show(L.Note), "note");
        assert.equal(L.show(L.Warning), "warning");
        assert.equal(L.show(L.Error), "error");
    });
});

suite("DiagnosticEmitter", () => {
    let engine = new D.DiagnosticEngine();
    let result = [];
    let loc = new D.DiagnosticLocation("", 0, "", 0, 0);
    engine.consumer = {
        handle: diag => {
            for (const msg of diag.messages) {
                let level = D.DiagnosticLevel.show(msg.level);
                result.push(`${level}: ${msg.format()} (with ${msg.args})`);
            }
        },
    }
    beforeEach(() => {
        result = [];
    });
    test("SimpleDiagnostic", () => {
        D.DiagnosticId.TestDiagnostic = {
            format: "hello, world",
        };
        engine.emit(
            D.Diagnostic.error(loc, D.DiagnosticId.TestDiagnostic));
        assert.deepEqual(result, [
            "error: hello, world (with undefined)",
        ]);
    });
    test("OneArgumentDiagnostic", () => {
        D.DiagnosticId.TestDiagnostic = {
            format: "the argument is `{}`",
        };
        engine.emit(
            D.Diagnostic.warning(loc, D.DiagnosticId.TestDiagnostic, ["hello"]));
        assert.deepEqual(result, [
            "warning: the argument is `hello` (with hello)",
        ]);
    });
    test("Note", () => {
        D.DiagnosticId.TestDiagnostic = {
            format: "hello, world",
        };
        D.DiagnosticId.TestNote = {
            format: "this is a note",
        };
        engine.emit(
            D.Diagnostic.error(loc, D.DiagnosticId.TestDiagnostic, ["main", "foo"])
                .note(loc, D.DiagnosticId.TestNote, ["n", "n2"]));
        assert.deepEqual(result, [
            "error: hello, world (with main,foo)",
            "note: this is a note (with n,n2)",
        ]);
    });
});
