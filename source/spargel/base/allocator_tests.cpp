#include "spargel/base/allocator.h"
#include "spargel/base/check.h"
#include "spargel/base/test.h"

//
#include <stdlib.h>

namespace spargel::base {
    namespace {
        class DummyAlloc final : public Allocator {
        public:
            ~DummyAlloc() { spargel_check(count_ == 0); }

            void* allocate(usize size) override {
                count_ += size;
                return ::malloc(size);
            }
            void* resize(void* ptr, usize old_size, usize new_size) override {
                spargel_check(count_ >= old_size);
                count_ = count_ - old_size + new_size;
                return ::realloc(ptr, new_size);
            }
            void free(void* ptr, usize size) override {
                spargel_check(count_ >= size);
                count_ -= size;
                ::free(ptr);
            }

        private:
            usize count_ = 0;
        };
        TEST(Allocator_Basic) {
            DummyAlloc alloc;
            Allocator* ptr = &alloc;
            auto p = ptr->allocObject<int>(3);
            ptr->freeObject(p);
        }
        TEST(Allocator_Resize) {
            Allocator* alloc = new DummyAlloc;

            auto p = alloc->allocate(1024);
            p = alloc->resize(p, 1024, 2048);
            alloc->free(p, 2048);

            p = alloc->allocate(1024);
            p = alloc->resize(p, 1024, 512);
            alloc->free(p, 512);
        }
    }  // namespace
}  // namespace spargel::base
