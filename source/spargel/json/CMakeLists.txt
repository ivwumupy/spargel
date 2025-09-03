spargel_add_library(
    NAME
        json
    PRIVATE
        cursor.cpp
        json_parser.cpp
    DEPS
        base
)

spargel_add_executable(
    NAME dump_json
    PRIVATE dump_json.cpp
    DEPS
        json
        resource
)

spargel_add_executable(
    NAME json_tests
    PRIVATE
        json_tests.cpp
    DEPS
        json
        test_main
)
add_test(
    NAME json_tests
    COMMAND json_tests
)
