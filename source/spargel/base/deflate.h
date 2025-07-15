#pragma once

#include <spargel/base/span.h>
#include <spargel/base/types.h>
#include <spargel/base/unique_ptr.h>
#include <spargel/base/vector.h>

namespace spargel::base {
    class DeflateDecompressor {
    public:
        static UniquePtr<DeflateDecompressor> make() { return make_unique<DeflateDecompressor>(); }

        void decompress(Span<Byte> input, Vector<Byte>& output);

    private:
        // The DEFLATE Format
        //
        // Each block starts with:
        //   1bit BFINAL - whether this is the last block
        //   2bit BTYPE  - compression type of the block
        //                   00 no compression
        //                   01 fixed Huffman
        //                   10 dynamic Huffman
        // Note that a block may not start at byte boundary.
        //
        // Code length must be in the range [0, 15], see [RFC1951, Section 3.2.7].
        // A decode table with 2^15 (= 32768) entries is a bit too large (not cache friendly?).
        // Thus we need (dynamically allocated) subtables.

        class BitStream {
        public:
            BitStream() = default;
            BitStream(u8 const* begin, u8 const* end) : next_{begin}, end_{end} {}

            void refill();
            void advance();

        private:
            // equal to the number of bits of the buffer
            static constexpr u8 MAX_BITS_LEFT = 64;

            u8 const* next_ = nullptr;
            u8 const* end_ = nullptr;
            u64 buffer_ = 0;
            u8 bits_left_ = 0;
        };

        void decompressBlock(Vector<Byte>& output);

        BitStream stream_;

        // Table sizes
        static constexpr usize LITLEN_TABLE_SIZE = 2342;

        // Decode tables
        u32 litlen_table_[LITLEN_TABLE_SIZE];
    };
}  // namespace spargel::base
