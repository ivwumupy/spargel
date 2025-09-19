#pragma once

#include "spargel/base/inline_array.h"
#include "spargel/base/result.h"
#include "spargel/base/types.h"

namespace spargel::gpu {
    namespace detail {
        inline u8 countLeadingZero(u64 x) {
            spargel_check(x != 0);
            return static_cast<u8>(__builtin_clzl(x));
        }
        inline u8 countTrailingZero(u64 x) {
            spargel_check(x != 0);
            return static_cast<u8>(__builtin_ctzl(x));
        }
        inline u8 countTrailingZero(u32 x) {
            spargel_check(x != 0);
            return static_cast<u8>(__builtin_ctz(x));
        }
    }  // namespace detail
    // The TLSF Allocator
    //
    // One instance manages a sequence of memory blocks, which are binned into a two levels.
    //
    // The number of (first level) bins is `bin_count`. Each bin contains `subbin_count` many
    // (second level) sub-bins. Every sub-bin contains a sequence of memory blocks, organized as a
    // free-list.
    //
    // Suppose the i-th (first level) bin is for memory blocks with size in [a, b).
    // Let delta be ceil((b - a) / subbin_count).
    // Then the j-th sub-bin is for blocks with size in [a + j * delta, a + (j + 1) * delta).
    //
    // We maintain two lookup structures:
    // - The first one is a bitset indicating whether each bin contains free memory blocks.
    // - The second one is a bitset for every bin, indicating which sub-bins contains free blocks.
    //
    // It remains to have a good choice of (first level) bins. A common approach is to associate
    // [2^i, 2^(i+1)) with the i-th bin. Zero-sized allocations are special handled. However, this
    // is not ideal for small sized bins.
    //
    // Therefore, we use the following ranges:
    // - 0-th bin: [2^0, 2^linear_bits)
    // - 1-th bin: [2^linear_bits, 2^(linear_bits + 1))
    // - 2-th bin: ...
    //
    // Then delta for sub-bins in the 0-th bin is (2^linear_bits - 1) / subbin_count, which is
    // about 3.9.
    //
    // Note: We would like 2^linear_bits to be divisible by subbin_count. So we take subbin_count to
    // be a power of 2. Moreover, we require that subbin_count <= 2^(linear_bits - 2) so that delta
    // >= 4.
    //
    //
    // size   | [2^0, 2^8) | [2^8, 2^9) | [2^9, 2^10) | ... | [2^63, 2^64)
    // bin_id |     0      |      1     |      2      | ... |
    //
    class TlsfAllocator {
    public:
        // The first bin has size `2 ** linear_bits`.
        // TODO: Maybe this should be 7.
        static constexpr u8 linear_bits = 8;
        static constexpr u64 min_alloc_size = u64(1) << linear_bits;
        static constexpr u8 subbin_bits = 5;
        // The number of subbin every bin holds.
        static constexpr u8 subbin_count = 1 << subbin_bits;
        // This is the number of total bins, including the linear one (which is the first one).
        // NOTE: The maximal size is `2 ** 64`.
        static constexpr u8 bin_count = 64 - linear_bits + 1;

        // the minimal alignment is 8
        static constexpr usize min_align = 1 << (linear_bits - subbin_bits);

        struct BinResult {
            u8 bin_id;
            u8 subbin_id;
            usize index() const { return bin_id * subbin_count + subbin_id; }
        };

        struct MemoryBlock {
            u64 offset;
            u64 size;
            // The previous block in the free list.
            MemoryBlock* prev_free;
            MemoryBlock* next_free;
            // The previous block that is physically adjacent.
            MemoryBlock* prev_phys;
            MemoryBlock* next_phys;
            bool is_free;
        };

        struct AllocatedMemory {
            MemoryBlock* block;
        };

        enum class AllocationError {
            // `size` must be greater than `min_align`.
            bad_size,
            // `align` is not a power of two.
            bad_align,
            // cannot find a free block
            out_of_block,
            unknown,
        };

        base::Result<AllocatedMemory, AllocationError> allocate(usize size, usize align) {
            if (size < min_align) {
                return base::Error{AllocationError::bad_size};
            }
            if ((align & (align - 1)) != 0) {
                return base::Error{AllocationError::bad_align};
            }
            auto bin = findBinWithFreeBlock(size);
            if (bin.is_error()) {
                return base::move(bin).error();
            }
            auto block = retrieveFreeBlock(bin.value());
            // TODO: try to split block
            return AllocatedMemory{block};
        }

        // Find the smallest (sub)bin larger than the given size.
        //
        // Implementation detail:
        // First we have the unadjusted bin id. It is valued in [8, 64). Based on
        // this id we know the subbin size `2 ** bin_id / subbin_count`. Since
        // `subbin_count` is a power of 2, the subbin size can be written as
        // `2 ** (bin_id - subbin_bits)`.
        static BinResult binUp(usize size) {
            spargel_check(size != 0);
            // The unadjusted id in the range [8, 64)
            auto bin_id = 63 - detail::countLeadingZero(size | min_alloc_size);
            // log_2 of the size of each subbin in the chosen bin.
            auto log2_subbin_size = bin_id - subbin_bits;
            // size of each subbin - 1
            auto delta = (u64(1) << log2_subbin_size) - 1;
            // TODO: Wrapped or saturated?
            auto rounded = size + delta;
            auto subbin_id = rounded >> log2_subbin_size;

            auto adjusted_bin_id = u64(bin_id - linear_bits) + (subbin_id >> subbin_bits);
            // Equivalent: `subbin_id % subbin_count`
            auto adjusted_subbin_id = subbin_id & (subbin_count - 1);

            spargel_check(adjusted_bin_id < bin_count);
            spargel_check(adjusted_subbin_id < subbin_count);

            return BinResult{u8(adjusted_bin_id), u8(adjusted_subbin_id)};
        }

        // Similar to `binUp`. The only difference is that we round `size` down instead of up.
        static BinResult binDown(usize size) {
            spargel_check(size != 0);
            // The unadjusted id in the range [8, 64)
            auto bin_id = 63 - detail::countLeadingZero(size | min_alloc_size);
            // log_2 of the size of each subbin in the chosen bin.
            auto log2_subbin_size = bin_id - subbin_bits;
            auto subbin_id = size >> log2_subbin_size;

            auto adjusted_bin_id = u64(bin_id - linear_bits) + (subbin_id >> subbin_bits);
            // Equivalent: `subbin_id % subbin_count`
            auto adjusted_subbin_id = subbin_id & (subbin_count - 1);

            spargel_check(adjusted_bin_id < bin_count);
            spargel_check(adjusted_subbin_id < subbin_count);

            return BinResult{u8(adjusted_bin_id), u8(adjusted_subbin_id)};
        }

        // Find a bin that contains a free block with enough space.
        base::Result<BinResult, AllocationError> findBinWithFreeBlock(usize size) const {
            auto [bin_id, subbin_id] = binUp(size);
            // Filter out subbins with enough space.
            auto avail_subbin = subbin_bitmaps_[bin_id] & (0xFFFFFFFF << subbin_id);
            if (avail_subbin == 0) {
                // In this case, no subbin in this bin contains a free block.
                // Then we should find the next free bin.
                auto avail_bin = bin_bitmap_ & (0xFFFFFFFF << (bin_id + 1));
                if (avail_bin == 0) {
                    return base::Error{AllocationError::out_of_block};
                }
                bin_id = detail::countTrailingZero(avail_bin);
                avail_bin = subbin_bitmaps_[bin_id];
                spargel_check(avail_subbin != 0);
            }
            subbin_id = detail::countTrailingZero(avail_subbin);
            return BinResult{bin_id, subbin_id};
        }

    private:
        // it is expected that the given bin is non-empty
        MemoryBlock* retrieveFreeBlock(BinResult bin) {
            auto index = bin.index();
            auto block = free_blocks_[index];
            spargel_check(block != nullptr);
            spargel_check(block->is_free);
            spargel_check(block->prev_free == nullptr);
            auto next = block->next_free;
            if (next != nullptr) {
                free_blocks_[index] = next;
                next->prev_free = nullptr;
            } else {
                // this subbin is now empty
                subbin_bitmaps_[bin.bin_id] &= ~(u32(1) << bin.subbin_id);
                if (subbin_bitmaps_[bin.bin_id] == 0) {
                    // this bin is empty
                    bin_bitmap_ &= ~(u32(1) << bin.bin_id);
                }
            }
            return block;
        }

        void insertFreeBlock(MemoryBlock* block) {
            auto bin = binDown(block->size);
            usize list_id = bin.bin_id * subbin_count + bin.subbin_id;
            addBlockToList(list_id, block);
            bin_bitmap_ |= u32(1) << bin.bin_id;
            subbin_bitmaps_[bin.bin_id] |= u32(1) << bin.subbin_id;
        }

        void addBlockToList(usize list_id, [[maybe_unused]] MemoryBlock* block) {
            spargel_check(block != nullptr);
            spargel_check(list_id < bin_count * subbin_count);
            auto head = free_blocks_[list_id];
            spargel_check(head->prev_free == nullptr);
            block->next_free = head;
            head->prev_free = block;
            free_blocks_[list_id] = block;
        }

        // NOTE: We support up to 64 bins.
        static_assert(bin_count <= 64);
        u64 bin_bitmap_;
        // NOTE: `subbin_count` is 32.
        static_assert(subbin_count <= 32);
        base::InlineArray<u32, bin_count> subbin_bitmaps_;
        base::InlineArray<MemoryBlock*, bin_count * subbin_count> free_blocks_;
    };

}  // namespace spargel::gpu

// This is the allocator for device memory.
//
// The maximal memory allocation count (maxMemoryAllocationCount) returned by Vulkan drivers is
// still 4096 on half of the devices. So we should have few allocations of large device memory
// blocks, and then suballocate from these blocks. It's then our task to handle memory
// fragmentation.
//
// Two algorithms:
// - Bump allocator.
// - TLSF (two-level seggregated fit) allocator.
//
