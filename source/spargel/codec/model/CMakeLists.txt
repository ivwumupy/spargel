spargel_add_library(
    NAME codec_gltf
    PRIVATE
        gltf.cpp
    DEPS
        codec_json
)

spargel_add_executable(
    NAME dump_gltf
    PRIVATE
        dump_gltf.cpp
    DEPS
        codec_gltf
)
