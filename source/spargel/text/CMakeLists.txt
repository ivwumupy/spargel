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

if (SPARGEL_ENABLE_FREETYPE)
    target_sources(text 
        PRIVATE font_freetype.cpp)
    target_include_directories(text
        PRIVATE ${FREETYPE_INCLUDE_DIRS}
    )
    target_link_libraries(text
        PRIVATE ${FREETYPE_LIBRARIES}
    )
endif ()

if (SPARGEL_ENABLE_HARFBUZZ)
    target_sources(text
        PRIVATE text_shaper_harfbuzz.cpp)
    target_link_libraries(text
        PRIVATE harfbuzz
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

if (SPARGEL_ENABLE_FREETYPE AND SPARGEL_ENABLE_HARFBUZZ)
    spargel_add_executable(
        NAME demo_font_console
        PRIVATE
            demo_font_console.cpp
        DEPS
            base
            text
    )
    target_include_directories(demo_font_console
        PRIVATE ${FREETYPE_INCLUDE_DIRS}
    )
endif ()
