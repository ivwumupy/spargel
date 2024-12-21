#include <spargel/base/allocator.h>
#include <spargel/base/test.h>
#include <spargel/base/vector.h>

// stl
#include <vector>

// from EASTL
struct MovableType {
    int8_t* mpData;
    enum { kDataSize = 128 };

    MovableType() : mpData(new int8_t[kDataSize]) { memset(mpData, 0, kDataSize); }

    MovableType(const MovableType& x) : mpData(new int8_t[kDataSize]) {
        memcpy(mpData, x.mpData, kDataSize);
    }

    MovableType& operator=(const MovableType& x) {
        if (!mpData) mpData = new int8_t[kDataSize];
        memcpy(mpData, x.mpData, kDataSize);
        return *this;
    }

    MovableType(MovableType&& x) : mpData(x.mpData) { x.mpData = NULL; }

    MovableType& operator=(MovableType&& x) {
        spargel::base::swap(
            mpData,
            x.mpData);  // In practice it may not be right to do a swap, depending on the case.
        return *this;
    }

    ~MovableType() { delete[] mpData; }
};

int main() {
    spargel::base::runBench(
        "std::vector::push", {1, 10, 100, 1000, 10000, 100000, 1000000},
        [](usize n) {
            std::vector<usize> v;
            for (usize i = 0; i < n; i++) {
                v.emplace_back(i);
            }
        },
        [](usize n, double t) { return t / n; });
    spargel::base::runBench(
        "base::vector::push", {1, 10, 100, 1000, 10000, 100000, 1000000},
        [](usize n) {
            spargel::base::vector<usize> v;
            for (usize i = 0; i < n; i++) {
                v.push(i);
            }
        },
        [](usize n, double t) { return t / n; });
    spargel::base::runBench(
        "base::vector::push(arena)", {1, 10, 100, 1000, 10000, 100000, 1000000},
        [](usize n) {
            auto arena = spargel::base::ArenaAllocator(n * sizeof(usize) * 10,
                                                       spargel::base::default_allocator());
            spargel::base::vector<usize> v(&arena);
            for (usize i = 0; i < n; i++) {
                v.push(i);
            }
        },
        [](usize n, double t) { return t / n; });

    spargel::base::runBench(
        "std::vector::push(movable)", {1, 10, 100, 1000, 10000},
        [](usize n) {
            std::vector<MovableType> v;
            for (usize i = 0; i < n; i++) {
                v.emplace_back();
            }
        },
        [](usize n, double t) { return t / n; });
    spargel::base::runBench(
        "base::vector::push(movable)", {1, 10, 100, 1000, 10000},
        [](usize n) {
            spargel::base::vector<MovableType> v;
            for (usize i = 0; i < n; i++) {
                v.push();
            }
        },
        [](usize n, double t) { return t / n; });
    spargel::base::runBench(
        "base::vector::push(movable,arena)", {1, 10, 100, 1000, 10000},
        [](usize n) {
            auto arena = spargel::base::ArenaAllocator(n * sizeof(MovableType) * 10,
                                                       spargel::base::default_allocator());
            spargel::base::vector<MovableType> v(&arena);
            for (usize i = 0; i < n; i++) {
                v.push();
            }
        },
        [](usize n, double t) { return t / n; });
}
