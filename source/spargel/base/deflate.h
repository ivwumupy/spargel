#pragma once

#include <spargel/base/check.h>
#include <spargel/base/checked_convert.h>
#include <spargel/base/span.h>
#include <spargel/base/types.h>
#include <spargel/base/unique_ptr.h>
#include <spargel/base/vector.h>

namespace spargel::base {
    class DeflateDecompressor {
    public:
        static UniquePtr<DeflateDecompressor> make() { return makeUnique<DeflateDecompressor>(); }

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

            // Fill the bit buffer as much as possible.
            void refill();
            // Advance n bits.
            //
            // NOTE: It's assumed that the buffer has enough bits.
            void advanceBits(u8 n) {
                spargel_check(n <= bits_left_);
                bits_left_ -= n;
                buffer_ >>= n;
            }

            // Align the stream to the next byte boundary.
            //
            // NOTE: The bit buffer will be cleared.
            void alignToBoundary();
            // Align to the next byte boundary and copy n bytes to the output.
            void alignAndCopy(usize n, Vector<Byte>& output) {
                alignToBoundary();
                copy(n, output);
            }

            void advanceBytes(usize n) {
                spargel_check(bits_left_ == 0);
                spargel_check(n <= checkedConvert<usize>(end_ - next_));
                next_ += n;
            }
            // Read (little-endian) u16 from the stream.
            //
            // NOTE: This is unaligned load.
            // TODO: Benchmark.
            u16 readU16() {
                return (static_cast<u16>(next_[1]) << 8) | static_cast<u16>(next_[0]);
            }

            u8 bit0() { return buffer_ & 0b1; }
            u8 bit21() { return (buffer_ >> 1) & 0b11; }

        private:
            // equal to the number of bits of the buffer
            static constexpr u8 MAX_BITS_LEFT = 64;
            // How many bits that are consumable regardless of bits left.
            // It's equal to MAX_BITS_LEFT - 7, otherwise a new byte can be filled.
            static constexpr u8 CONSUMABLE_BITS = MAX_BITS_LEFT - 7;

            // Copy n bytes to output.
            void copy(usize n, Vector<Byte>& output);

            u8 const* next_ = nullptr;
            u8 const* end_ = nullptr;
            // several bytes
            u64 buffer_ = 0;
            // bits remaining in the buffer
            u8 bits_left_ = 0;
            // The count of virtual zero bytes added at the end.
            //
            // Invariant:
            //   trailing_zero_ <= sizeof(buffer_) = 8
            u8 trailing_zero_ = 0;
        };

        void decompressBlock(Vector<Byte>& output);
        void plainBlock(Vector<Byte>& out);

        BitStream stream_;

        // Table sizes
        static constexpr usize LITLEN_TABLE_SIZE = 2342;

        // Decode tables
        u32 litlen_table_[LITLEN_TABLE_SIZE];
    };
}  // namespace spargel::base
