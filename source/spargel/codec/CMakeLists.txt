spargel_add_library(
    NAME codec
    PRIVATE
        json_codec.cpp
    DEPS
        base
        json
)

# TEST

spargel_add_executable(
    NAME test_codec
    PRIVATE test_codec.cpp
    DEPS
        codec
        test_main
)
add_test(
    NAME test_codec
    COMMAND test_codec
)

#spargel_add_executable(
#    NAME test_json_codec
#    PRIVATE test_json_codec.cpp
#    DEPS
#        codec_json
#        test_main
#)
#add_test(
#    NAME test_json_codec
#    COMMAND test_json_codec
#)

# OTHER

add_subdirectory(model/)
