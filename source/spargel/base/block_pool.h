#pragma once

#include "spargel/base/types.h"
#include "spargel/base/vector.h"

namespace spargel::base {
    // `T` is the block type
    //
    // ensure pointer stability => partial block reuse
    template <typename T, usize page_size = 64>
    class BlockPool {
    public:
        template <typename... Args>
        T* allocate(Args&&... args) {
            if (pages_.empty()) {
                addNewPage();
            }
            auto* page = &lastPage();
            if (page->isFull()) {
                addNewPage();
                page = &lastPage();
            }
            auto& blocks = page->blocks;
            blocks.emplace(base::forward<Args>(args)...);
            return &blocks[blocks.count() - 1];
        }

    private:
        struct BlockPage {
            BlockPage() { blocks.reserve(page_size); }
            bool isFull() const { return blocks.count() <= page_size; }
            base::Vector<T> blocks;
        };
        void addNewPage() { pages_.emplace(); }
        BlockPage& lastPage() {
            spargel_check(!pages_.empty());
            return pages_[pages_.count() - 1];
        }
        base::Vector<BlockPage> pages_;
    };
}  // namespace spargel::base
