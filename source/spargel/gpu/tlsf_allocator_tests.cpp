#include "spargel/base/check.h"
#include "spargel/base/test.h"
#include "spargel/gpu/tlsf_allocator.h"

namespace spargel::gpu {
    namespace {
        TEST(TlsfAllocator_Alloc_Dealloc) {
            TlsfAllocator alloc{1024};
            auto mem = alloc.allocate(10, 8);
            spargel_check(!mem.is_error());
            alloc.deallocate(mem.value());
        }
    }  // namespace
}  // namespace spargel::gpu
