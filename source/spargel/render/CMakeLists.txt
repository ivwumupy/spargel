
if (${SPARGEL_IS_MACOS})

spargel_add_executable(
    NAME demo_metal
    PRIVATE_MACOS
        demo_metal.mm
)

endif ()
