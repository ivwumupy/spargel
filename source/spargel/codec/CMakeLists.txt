spargel_add_library(
    NAME codec
    PRIVATE
        codec.cpp
        cursor.cpp
    DEPS
        base
)

if (0)
spargel_add_library(
    NAME codec_json
    PRIVATE
        json_parser.cpp
        json_utils.cpp
    DEPS
        codec
)
endif ()

# TEST

spargel_add_executable(
    NAME test_codec
    PRIVATE test_codec.cpp
    DEPS codec
)

if (0)
spargel_add_executable(
    NAME test_json
    PRIVATE test_json.cpp
    DEPS codec_json
)
endif ()

# OTHER

if (0)
spargel_add_executable(
    NAME dump_json
    PRIVATE dump_json.cpp
    DEPS codec_json
)
endif ()

if (0)
add_subdirectory(model/)
endif ()
