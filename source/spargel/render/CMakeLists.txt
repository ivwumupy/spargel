if (SPARGEL_ENABLE_OPENGL)
    spargel_add_executable(
        NAME demo_glad
        PRIVATE
            demo_glad.c
    )
    target_link_libraries(demo_glad PRIVATE glad)
endif()

if (SPARGEL_IS_MACOS)
    spargel_add_executable(
        NAME demo_metal
        PRIVATE_MACOS
            demo_metal.mm
        DEPS
            base
            resource
            ui
    )
    spargel_application_add_resources(
        TARGET demo_metal
        SRC ${CMAKE_CURRENT_SOURCE_DIR}/demo_assets
        DST
        DEPS
    )

    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/shader/shader.ir
        COMMAND
            xcrun metal -frecord-sources -o ${CMAKE_CURRENT_BINARY_DIR}/shader/shader.ir -c ${CMAKE_CURRENT_SOURCE_DIR}/shader/shader.metal
        DEPENDS shader/shader.metal
    )
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/shader/shader.metallib
        COMMAND
            xcrun metallib -o ${CMAKE_CURRENT_BINARY_DIR}/shader/shader.metallib ${CMAKE_CURRENT_BINARY_DIR}/shader/shader.ir
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/shader/shader.ir
    )
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/resources/shader.metallib
        COMMAND
            cp ${CMAKE_CURRENT_BINARY_DIR}/shader/shader.metallib ${CMAKE_CURRENT_BINARY_DIR}/resources/shader.metallib
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/shader/shader.metallib
    )
    add_custom_target(demo_render_metal_shaders
        DEPENDS
            ${CMAKE_CURRENT_BINARY_DIR}/resources/shader.metallib
    )
    add_dependencies(demo_metal demo_render_metal_shaders)
endif ()
