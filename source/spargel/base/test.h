#pragma once

#include <spargel/base/allocator.h>
#include <spargel/base/object.h>
#include <spargel/base/string_view.h>
#include <spargel/base/vector.h>

// libc
#include <math.h>
#include <stdio.h>

// platform
#if SPARGEL_IS_MACOS
#include <time.h>
#endif

namespace spargel::base {

    class Test {
    public:
        virtual ~Test() = default;
        virtual void run() = 0;
    };

    class TestManager {
    public:
        static TestManager* instance();

        template <typename T>
        T* registerTest(char const* name) {
            T* ptr = static_cast<T*>(default_allocator()->alloc(sizeof(T)));
            construct_at<T>(ptr);
            _tests.push(name, ptr);
            return ptr;
        }

        void runAll();

    private:
        struct TestEntry {
            char const* name;
            Test* test;
        };

        vector<TestEntry> _tests;
    };

    void runBench(string_view name, span<usize> variants, auto&& f, auto&& op) {
        base::vector<double> results;
        for (usize i = 0; i < variants.count(); i++) {
            auto v = variants[i];
            const int warmup_number = 100;
            const int run_number = 10;
            printf("%-50s/%-10lu", name.data(), v);
            for (int j = 0; j < warmup_number; j++) {
                f(v);
            }
#if SPARGEL_IS_MACOS
            auto start = clock_gettime_nsec_np(CLOCK_UPTIME_RAW);
            for (int j = 0; j < run_number; j++) {
                f(v);
            }
            auto end = clock_gettime_nsec_np(CLOCK_UPTIME_RAW);
            auto average = static_cast<double>(end - start) / run_number;
            auto per_op = op(v, average);
            printf("%-10.2f ns    %-10.2f ns per op\n", average, per_op);
#endif
        }
    }

}  // namespace spargel::base

#define _TEST_CLASS_NAME(test_name) _SpargelTestClass_##test_name

#define TEST(test_name)                                                                      \
    class _TEST_CLASS_NAME(test_name) final : public ::spargel::base::Test {                 \
    public:                                                                                  \
        void run() override;                                                                 \
                                                                                             \
    private:                                                                                 \
        static _TEST_CLASS_NAME(test_name) * _instance;                                      \
    };                                                                                       \
    _TEST_CLASS_NAME(test_name) * _TEST_CLASS_NAME(test_name)::_instance =                   \
        ::spargel::base::TestManager::instance()->registerTest<_TEST_CLASS_NAME(test_name)>( \
            #test_name);                                                                     \
    void _TEST_CLASS_NAME(test_name)::run()
