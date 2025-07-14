#include <spargel/base/coroutine.h>
#include <spargel/base/meta.h>

//
#include <stdio.h>

namespace spargel::base {
    namespace {
        // the object returned to the caller when the coroutine is initially suspended
        struct return_object {
            struct promise_type {
                return_object get_return_object() {
                    printf("0\n");
                    return {};
                }
                NeverSuspend initial_suspend() {
                    printf("1\n");
                    return {};
                }
                AlwaysSuspend final_suspend() noexcept { return {}; }
                void return_void() {}
                void unhandled_exception() {}
            };
        };
        struct awaitable {
            CoroutineHandle<>* handle;
            // false => result not ready => the coroutine is suspended
            bool await_ready() {
                printf("3\n");
                return false;
            }
            // return void => transfer control to the caller/resumer
            void await_suspend(CoroutineHandle<> h) {
                // schedule the (suspended) coroutine to some executor to execute later
                *handle = h;
            }
            void await_resume() {
                printf("resume\n");
            }
        };
        return_object counter(CoroutineHandle<>* handle) {
            awaitable a{handle};
            printf("2\n");
            co_await a;
            printf("n\n");
            co_await a;
            co_return;
        }
        void test_coroutine() {
            CoroutineHandle<> handle;
            [[maybe_unused]] auto h = counter(&handle);
            printf("x\n");
            handle();
            printf("10\n");
            handle();
            printf("20\n");
        }
        template <typename T>
        struct Generator {
            struct promise_type;
            using Handle = CoroutineHandle<promise_type>;
            struct promise_type {
                T value;
                Generator<T> get_return_object() {
                    return {Handle::from_promise(*this)};
                }
                AlwaysSuspend initial_suspend() { return {}; }
                AlwaysSuspend final_suspend() noexcept { return {}; }
                void return_void() {}
                template <typename U>
                AlwaysSuspend yield_value(U&& v) {
                    value = forward<U>(v);
                    return {};
                }
                void unhandled_exception() {}
            };

            ~Generator() {
                handle.destroy();
            }

            bool done() const { return handle.done(); }

            T operator()() {
                handle.resume();
                return move(handle.promise().value);
            }

            CoroutineHandle<promise_type> handle;
            bool full = false;
        };
        Generator<int> iota(int m = 0) {
            int x = m;
            while (true) {
                co_yield x;
                x += 1;
            }
        }
        void test_generator() {
            auto gen = iota(3);
            for (int i = 0; i < 10; i++) {
                printf("iota: %d -> %d\n", i, gen());
            }
        }
    }  // namespace
}  // namespace spargel::base

int main() {
    spargel::base::test_coroutine();
    spargel::base::test_generator();
    return 0;
}
