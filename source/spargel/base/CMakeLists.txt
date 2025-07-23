spargel_add_library(
    NAME base
    PRIVATE
        allocator.cpp
        deflate.cpp
        panic.cpp
        platform.cpp
        string.cpp
        logging.cpp
        task.cpp
        test.cpp
        trace.cpp
    PRIVATE_POSIX
        platform_posix.cpp
    PRIVATE_ANDROID
        platform_linux.cpp
    PRIVATE_EMSCRIPTEN
        platform_emscripten.cpp
    PRIVATE_LINUX
        platform_linux.cpp
    PRIVATE_MACOS
        platform_mac.cpp
    PRIVATE_WINDOWS
        platform_win.cpp
    DEPS spargel_config
)

spargel_add_library(
    NAME test_main
    PRIVATE
        test_main.cpp
    DEPS base
)

if (SPARGEL_IS_LINUX OR SPARGEL_IS_ANDROID)
    target_link_libraries(base PUBLIC m)
endif ()

if (SPARGEL_ENABLE_LIBUNWIND)
    target_link_libraries(base
        PUBLIC
            unwind
            unwind-${CMAKE_SYSTEM_PROCESSOR}
    )
endif ()

# DEMO

spargel_add_executable(
    NAME backtrace_demo
    PRIVATE backtrace_demo.cpp
    DEPS base
)

spargel_add_executable(
    NAME executable_path_demo
    PRIVATE executable_path_demo.cpp
    DEPS base
)

spargel_add_executable(
    NAME log_demo
    PRIVATE log_demo.cpp
    DEPS base
)

spargel_add_executable(
    NAME panic_demo
    PRIVATE panic_demo.cpp
    DEPS base
)

spargel_add_executable(
    NAME tag_invoke_demo
    PRIVATE tag_invoke_demo.cpp
    DEPS base
)

spargel_add_executable(
    NAME coroutine_demo
    PRIVATE coroutine_demo.cpp
    DEPS base
)

spargel_add_executable(
    NAME deflate_demo
    PRIVATE deflate_demo.cpp
    DEPS
        base
        resource
)

# TEST

spargel_add_executable(
  NAME base_tests
  PRIVATE
    array_storage_test.cpp
    either_test.cpp
    functional_test.cpp
    hash_map_test.cpp
    hash_test.cpp
    inline_array_test.cpp
    meta_test.cpp
    optional_test.cpp
    ref_ptr_tests.cpp
    string_test.cpp
    string_view_test.cpp
    sum_type_test.cpp
    tagged_union_test.cpp
    tuple_test.cpp
    vector_test.cpp
  DEPS
    base
    test_main
)
add_test(
    NAME base_tests
    COMMAND base_tests
)
