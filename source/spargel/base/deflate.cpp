#include <spargel/base/deflate.h>

namespace spargel::base {
    void DeflateDecompressor::decompress(Span<Byte> input, Vector<Byte>& out) {
        stream_ = BitStream(input.begin(), input.end());
        while (true) {
            // TODO
            decompressBlock(out);
        }
    }
    void DeflateDecompressor::decompressBlock(Vector<Byte>& out) {
        // TODO
    }
}  // namespace spargel::base
