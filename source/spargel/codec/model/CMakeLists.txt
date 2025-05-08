spargel_add_library(
    NAME codec_gltf
    PRIVATE
        gltf.cpp
    DEPS
        codec_json
)

spargel_add_library(
    NAME codec_gltf2
    PRIVATE
        gltf2.cpp
    DEPS
        codec_json
)

spargel_add_executable(
    NAME summary_gltf
    PRIVATE
        summary_gltf.cpp
    DEPS
        codec_gltf
        resource
)

spargel_add_executable(
    NAME summary_gltf2
    PRIVATE
        summary_gltf.cpp
    DEPS
        codec_gltf2
        resource
)
