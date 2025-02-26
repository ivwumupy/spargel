spargel_add_library(
    NAME codec
    PRIVATE
        codec.cpp
        cursor.cpp
        json2.cpp
    DEPS
        base
)

spargel_add_executable(
    NAME test_json
    PRIVATE test_json.cpp
    DEPS codec
)

spargel_add_executable(
    NAME dump_json2
    PRIVATE dump_json2.cpp
    DEPS codec
)

if (FALSE)
spargel_add_executable(
    NAME ppm_info
    PRIVATE ppm_info.cpp
    DEPS codec
)

spargel_add_executable(
    NAME dump_json
    PRIVATE dump_json.cpp
    DEPS codec
)
endif ()
