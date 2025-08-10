spargel_add_library(
    NAME render
    PRIVATE
        atlas_packer.cpp
        ui_renderer.cpp
        ui_scene.cpp
    PRIVATE_MACOS
        ui_renderer_metal.mm
    DEPS
        base
        gpu
        text
)

spargel_add_executable(
    NAME demo_ui_renderer
    PRIVATE
        demo_ui_renderer.cpp
    DEPS
        ui
        render
        resource
)

if (SPARGEL_IS_MACOS)
    spargel_add_executable(
        NAME headless_metal
        PRIVATE_MACOS
            headless_metal.mm
        DEPS
            render
    )

    add_custom_command(
        OUTPUT
            ${CMAKE_CURRENT_BINARY_DIR}/metal_shaders.h
            ${CMAKE_CURRENT_BINARY_DIR}/metal_shaders.cpp
        COMMAND
            python3 ${PROJECT_SOURCE_DIR}/gn/script/compile_metal.py
                --gen-dir ${CMAKE_BINARY_DIR}
                --output-base source/spargel/render
                --target-name metal_shaders
                --namespace "spargel::render"
                --var-name "METAL_SHADERS"
                --compile
                    ${CMAKE_CURRENT_SOURCE_DIR}/ui_shaders.metal
                    ui_shaders.metal.ir
        DEPENDS
            ui_shaders.metal
    )
    target_sources(render
        PRIVATE
            ${CMAKE_CURRENT_BINARY_DIR}/metal_shaders.h
            ${CMAKE_CURRENT_BINARY_DIR}/metal_shaders.cpp
    )
endif ()


if (SPARGEL_ENABLE_OPENGL)

    spargel_add_executable(
        NAME demo_opengl
        PRIVATE
            demo_opengl.cpp
        DEPS
            glad
            resource
            ui
    )

    spargel_add_executable(
        NAME demo_gltf_opengl
        PRIVATE
            demo_gltf_opengl.cpp
        DEPS
            codec_gltf
            glad
            resource
            ui
    )

    if (SPARGEL_IS_WINDOWS)
        find_package(OpenGL REQUIRED)
        target_link_libraries(demo_opengl PRIVATE OpenGL::GL)
        target_link_libraries(demo_gltf_opengl PRIVATE OpenGL::GL)
    endif ()

    add_custom_target(demo_opengl_shaders ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/resources
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/demo_opengl.vs ${CMAKE_CURRENT_BINARY_DIR}/resources/
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/demo_opengl.fs ${CMAKE_CURRENT_BINARY_DIR}/resources/
    )

    add_dependencies(demo_opengl demo_opengl_shaders)

endif ()
