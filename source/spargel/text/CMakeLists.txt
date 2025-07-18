spargel_add_library(
    NAME text
    PRIVATE
        font_manager.cpp
    PRIVATE_MACOS
        font_mac.cpp
        text_shaper_mac.cpp
    DEPS
        base
)

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
