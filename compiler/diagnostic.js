// Diagnostic

const DiagnosticLevel = {
    Note: 0,
    Warning: 1,
    Error: 2,

    show(x) {
        switch (x) {
            case this.Note:
                return "note";
            case this.Warning:
                return "warning";
            case this.Error:
                return "error";
        }
    },
};

const DiagnosticId = {
    InternalError: {
        format: "internal error",
    },
    InvalidCharacter: {
        format: "invalid character `{}`",
    },
};

function DiagnosticLocation(filename, line_number, line, start, end) {
    this.filename = filename;
    this.line_number = line_number;
    this.line = line;
    this.start = start;
    this.end = end;
}

function DiagnosticMessage(level, loc, id, args) {
    this.level = level;
    this.location = loc;
    this.id = id;
    this.args = args;
}

DiagnosticMessage.prototype.format = function () {
    let idx = 0;
    return this.id.format.replace(/{}/g, () => {
        let arg = this.args[idx];
        idx += 1;
        return arg;
    });
};

function Diagnostic(level, msgs) {
    this.level = level;
    this.messages = msgs;
}

Diagnostic.error = function (loc, id, args) {
    return new Diagnostic(DiagnosticLevel.Error, [
        new DiagnosticMessage(DiagnosticLevel.Error, loc, id, args),
    ]);
};

Diagnostic.warning = function (loc, id, args) {
    return new Diagnostic(DiagnosticLevel.Warning, [
        new DiagnosticMessage(DiagnosticLevel.Warning, loc, id, args),
    ]);
};

Diagnostic.prototype.note = function (loc, id, args) {
    this.messages.push(new DiagnosticMessage(DiagnosticLevel.Note, loc, id, args));
    return this;
};

function DiagnosticEngine() {}

DiagnosticEngine.prototype.consumer = {
    // diag: Diagnostic
    handle: diag => {
        for (const msg of diag.messages) {
            let level = DiagnosticLevel.show(msg.level);
            let loc = msg.location;
            console.log(`${level}: ${msg.format()}`);
            console.log(`  --> ${loc.filename}:${loc.line_number}:`);
            let line_number = loc.line_number.toString();
            let indent = ' '.repeat(line_number.length + 2);
            let indent2 = ' '.repeat(loc.start);
            let emph = '^'.repeat(loc.end - loc.start);
            console.log(`${indent}|`);
            console.log(` ${line_number} |${loc.line}`);
            console.log(`${indent}|${indent2}${emph}`);
        }
    },
};

DiagnosticEngine.prototype.emit = function (diag) {
    this.consumer.handle(diag);
};

module.exports = {
    DiagnosticLevel,
    DiagnosticId,
    DiagnosticLocation,
    DiagnosticMessage,
    Diagnostic,
    DiagnosticEngine,
};
