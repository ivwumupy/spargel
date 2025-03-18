const fs = require("node:fs");
const { Buffer } = require("node:buffer");

// `content` is a `Buffer`.
function SourceBuffer(filename, content) {
    this.filename = filename;
    this.content = content;
}

SourceBuffer.prototype.length = function () {
    return this.content.length;
}

SourceBuffer.loadFile = path => {
    // read the file as binary data
    let data = fs.readFileSync(path);
    return new SourceBuffer(path, data);
};

SourceBuffer.fromString = s => {
    return new SourceBuffer("<input>", Buffer.from(s, "utf8"));
};

module.exports = {
    SourceBuffer,
};

