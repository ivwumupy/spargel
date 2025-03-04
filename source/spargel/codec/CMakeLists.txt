spargel_add_library(
    NAME codec
    PRIVATE
        codec.cpp
        cursor.cpp
    DEPS
        base
)

spargel_add_library(
    NAME codec_json
    PRIVATE
        json_parser.cpp
        json_utils.cpp
    DEPS
        codec
)

spargel_add_executable(
    NAME test_json
    PRIVATE
        test_json.cpp
    DEPS
        codec_json
)

spargel_add_executable(
    NAME dump_json
    PRIVATE
        dump_json.cpp
    DEPS
        codec_json
)

spargel_add_executable(
    NAME try_codec
    PRIVATE
        try_codec.cpp
    DEPS
        base
)

add_subdirectory(model/)
