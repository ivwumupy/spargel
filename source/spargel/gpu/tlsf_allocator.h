#pragma once

#include "spargel/base/block_pool.h"
#include "spargel/base/inline_array.h"
#include "spargel/base/result.h"
#include "spargel/base/types.h"
#include "spargel/base/vector.h"

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
        // `align` must be a power of two
        inline usize alignForward(usize addr, usize align) {
            return (addr + (align - 1)) & ~(align - 1);
        }
    }  // namespace detail
    // The TLSF Allocator
    //
    // One instance manages a sequence of memory blocks, which are binned into a
    // two levels.
    //
    // The number of (first level) bins is `bin_count`. Each bin contains
    // `subbin_count` many (second level) sub-bins. Every sub-bin contains a
    // sequence of memory blocks, organized as a free-list.
    //
    // Suppose the i-th (first level) bin is for memory blocks with size in [a,
    // b). Let delta be ceil((b - a) / subbin_count). Then the j-th sub-bin is
    // for blocks with size in [a + j * delta, a + (j + 1) * delta).
    //
    // We maintain two lookup structures:
    // - The first one is a bitset indicating whether each bin contains free
    // memory blocks.
    // - The second one is a bitset for every bin, indicating which sub-bins
    // contains free blocks.
    //
    // It remains to have a good choice of (first level) bins. A common approach
    // is to associate [2^i, 2^(i+1)) with the i-th bin. Zero-sized allocations
    // are special handled. However, this is not ideal for small sized bins.
    //
    // Therefore, we use the following ranges:
    // - 0-th bin: [2^0, 2^linear_bits)
    // - 1-th bin: [2^linear_bits, 2^(linear_bits + 1))
    // - 2-th bin: ...
    //
    // Then delta for sub-bins in the 0-th bin is (2^linear_bits - 1) /
    // subbin_count, which is about 3.9.
    //
    // Note: We would like 2^linear_bits to be divisible by subbin_count. So we
    // take subbin_count to be a power of 2. Moreover, we require that
    // subbin_count <= 2^(linear_bits - 2) so that delta
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
        // This is the number of total bins, including the linear one (which is
        // the first one). NOTE: The maximal size is `2 ** 64`.
        static constexpr u8 bin_count = 64 - linear_bits + 1;

        // the minimal alignment is 8
        static constexpr usize min_align = 1 << (linear_bits - subbin_bits);

        struct MemoryBlock {
            u64 offset = 0;
            u64 size = 0;
            // The previous block in the free list.
            MemoryBlock* prev_free = nullptr;
            MemoryBlock* next_free = nullptr;
            // The previous block that is physically adjacent.
            MemoryBlock* prev_phys = nullptr;
            MemoryBlock* next_phys = nullptr;
            bool is_free = false;
        };

        struct AllocatedMemory {
            MemoryBlock* block = nullptr;
            usize offset = 0;
            usize align = 0;
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

        explicit TlsfAllocator(usize total_size) {
            auto block = new MemoryBlock;
            block->offset = 0;
            block->size = total_size;
            insertFreeBlock(block);
        }

        base::Result<AllocatedMemory, AllocationError> allocate(usize size,
                                                                usize align) {
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
            return allocateInBlock(block, size, align);
        }

        void deallocate(AllocatedMemory memory) {
            auto block = memory.block;
            block->is_free = true;
            insertFreeBlock(block);
        }

    private:
        struct BinResult {
            u8 bin_id;
            u8 subbin_id;
            usize index() const { return bin_id * subbin_count + subbin_id; }
        };

        // a helper class to manage `N` lists of free blocks
        template <usize N>
        class BlockManager {
        public:
            // insert a block to the `i`-th list
            void insert(usize i, MemoryBlock* block) {
                spargel_check(i < N);
                spargel_check(block != nullptr);
                auto* head = free_lists_[i];
                free_lists_[i] = block;
                block->next_free = head;
                block->prev_free = nullptr;
                if (head != nullptr) {
                    head->prev_free = block;
                }
            }

            // get a block from the `i`-th list
            MemoryBlock* consume(usize i) {
                spargel_check(i < N);
                auto* block = free_lists_[i];
                if (block == nullptr) {
                    return nullptr;
                }
                auto* next = block->next_free;
                if (next != nullptr) {
                    next->prev_free = nullptr;
                }
                block->next_free = nullptr;
                spargel_check(block->prev_free == nullptr);
                free_lists_[i] = next;
                return block;
            }

            bool empty(usize i) const {
                spargel_check(i < N);
                return free_lists_[i] == nullptr;
            }

            MemoryBlock* split(MemoryBlock* block, usize offset) {
                spargel_check(!block->is_free);
                auto* new_block = pool_.allocate();
                new_block->offset = block->size + offset;
                new_block->size = block->size - offset;
                block->size -= offset;
                new_block->prev_phys = block;
                new_block->next_phys = block->next_phys;
                block->next_phys = new_block;
                spargel_check(!new_block->is_free);
                return new_block;
            }

        private:
            // memory pool ensures pointer stability
            base::BlockPool<MemoryBlock> pool_;
            base::InlineArray<MemoryBlock*, N> free_lists_ = {};
        };

        // Find the smallest (sub)bin larger than the given size.
        //
        // Implementation detail:
        // First we have the unadjusted bin id. It is valued in [8, 64). Based
        // on this id we know the subbin size `2 ** bin_id / subbin_count`.
        // Since `subbin_count` is a power of 2, the subbin size can be written
        // as `2 ** (bin_id - subbin_bits)`.
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

            auto adjusted_bin_id =
                u64(bin_id - linear_bits) + (subbin_id >> subbin_bits);
            // Equivalent: `subbin_id % subbin_count`
            auto adjusted_subbin_id = subbin_id & (subbin_count - 1);

            spargel_check(adjusted_bin_id < bin_count);
            spargel_check(adjusted_subbin_id < subbin_count);

            return BinResult{u8(adjusted_bin_id), u8(adjusted_subbin_id)};
        }

        // Similar to `binUp`. The only difference is that we round `size` down
        // instead of up.
        static BinResult binDown(usize size) {
            spargel_check(size != 0);
            // The unadjusted id in the range [8, 64)
            auto bin_id = 63 - detail::countLeadingZero(size | min_alloc_size);
            // log_2 of the size of each subbin in the chosen bin.
            auto log2_subbin_size = bin_id - subbin_bits;
            auto subbin_id = size >> log2_subbin_size;

            auto adjusted_bin_id =
                u64(bin_id - linear_bits) + (subbin_id >> subbin_bits);
            // Equivalent: `subbin_id % subbin_count`
            auto adjusted_subbin_id = subbin_id & (subbin_count - 1);

            spargel_check(adjusted_bin_id < bin_count);
            spargel_check(adjusted_subbin_id < subbin_count);

            return BinResult{u8(adjusted_bin_id), u8(adjusted_subbin_id)};
        }

        // Find a bin that contains a free block with enough space.
        base::Result<BinResult, AllocationError> findBinWithFreeBlock(
            usize size) const {
            auto [bin_id, subbin_id] = binUp(size);
            // Filter out subbins with enough space.
            auto avail_subbin =
                subbin_bitmaps_[bin_id] & (~u32(0) << subbin_id);
            if (avail_subbin == 0) {
                // In this case, no subbin in this bin contains a free block.
                // Then we should find the next free bin.
                auto avail_bin = bin_bitmap_ & (~u64(0) << (bin_id + 1));
                if (avail_bin == 0) {
                    return base::Error{AllocationError::out_of_block};
                }
                bin_id = detail::countTrailingZero(avail_bin);
                avail_subbin = subbin_bitmaps_[bin_id];
                spargel_check(avail_subbin != 0);
            }
            subbin_id = detail::countTrailingZero(avail_subbin);
            return BinResult{bin_id, subbin_id};
        }

        AllocatedMemory allocateInBlock(MemoryBlock* block, usize size,
                                        usize align) {
            block->is_free = false;
            auto adjusted_offset = detail::alignForward(block->offset, align);
            auto padding = adjusted_offset - block->offset;
            auto size_with_align = size + padding;
            //    align padding
            // |........|----------allocated memory----------|...new block...|
            // ^ offset of block
            // TODO: do we need to add min_subbin_size
            if (block->size >= size_with_align + min_align) {
                auto* new_block = manager_.split(block, size_with_align);
                insertFreeBlock(new_block);
            }
            return AllocatedMemory{
                .block = block, .offset = adjusted_offset, .align = align};
        }

        // it is expected that the given bin is non-empty
        MemoryBlock* retrieveFreeBlock(BinResult bin) {
            auto index = bin.index();
            auto* block = manager_.consume(index);
            spargel_check(block != nullptr);
            if (manager_.empty(index)) {
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
            spargel_check(block != nullptr);

            auto bin = binDown(block->size);
            usize list_id = bin.bin_id * subbin_count + bin.subbin_id;
            manager_.insert(list_id, block);
            bin_bitmap_ |= u32(1) << bin.bin_id;
            subbin_bitmaps_[bin.bin_id] |= u32(1) << bin.subbin_id;
            block->is_free = true;
        }

        // NOTE: We support up to 64 bins.
        static_assert(bin_count <= 64);
        u64 bin_bitmap_ = 0;
        // NOTE: `subbin_count` is 32.
        static_assert(subbin_count <= 32);
        base::InlineArray<u32, bin_count> subbin_bitmaps_ = {};
        BlockManager<bin_count * subbin_count> manager_;
    };

}  // namespace spargel::gpu
