#include "spargel/base/allocator.h"

#include "spargel/base/check.h"

// libc
#include <stdlib.h>

namespace spargel::base {
    namespace {
        class LibCAllocator final : public Allocator {
        public:
            static LibCAllocator* getInstance();

            void* allocate(usize size) override;
            void* resize(void* ptr, usize old_size, usize new_size) override;
            void free(void* ptr, usize size) override;
        };
        LibCAllocator* LibCAllocator::getInstance() {
            static LibCAllocator* inst = new LibCAllocator();
            return inst;
        }

        void* LibCAllocator::allocate(usize size) {
            spargel_check(size > 0);
            return ::malloc(size);
        }

        void* LibCAllocator::resize(void* ptr, usize old_size, usize new_size) {
            spargel_check(ptr != nullptr);
            spargel_check(old_size > 0);
            spargel_check(new_size > 0);
            return ::realloc(ptr, new_size);
        }

        void LibCAllocator::free(void* ptr, usize size) {
            spargel_check(ptr != nullptr);
            spargel_check(size > 0);
            ::free(ptr);
        }
    }  // namespace

    Allocator* default_allocator() { return LibCAllocator::getInstance(); }

}  // namespace spargel::base
