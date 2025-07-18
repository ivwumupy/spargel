spargel_add_library(
    NAME text
    PRIVATE
        font_manager.cpp
    PRIVATE_LINUX
        font_freetype.cpp
        text_shaper_harfbuzz.cpp
    PRIVATE_MACOS
        font_mac.cpp
        text_shaper_mac.cpp
    DEPS
        base
)

if (SPARGEL_IS_LINUX)

    target_include_directories(text
        PRIVATE
            ${FREETYPE_INCLUDE_DIRS}
    )

    target_link_libraries(text
        PRIVATE
            harfbuzz
            resource
            ${FREETYPE_LIBRARIES}
    )

endif ()

if (SPARGEL_IS_MACOS)
    target_link_libraries(text
        PRIVATE
            ${FRAMEWORK_CORE_FOUNDATION}
            ${FRAMEWORK_CORE_GRAPHICS}
            ${FRAMEWORK_CORE_TEXT}
    )
endif ()

spargel_add_executable(
    NAME demo_font
    PRIVATE
        demo_font.cpp
    DEPS
        base
        text
)
