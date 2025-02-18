
if (SPARGEL_IS_MACOS)

  spargel_add_executable(
    NAME demo_metal
    PRIVATE_MACOS
      demo_metal.mm
  )

endif ()


if (SPARGEL_ENABLE_OPENGL)

  spargel_add_executable(
    NAME demo_glad
    PRIVATE
      demo_glad.c
  )
  target_link_libraries(demo_glad PRIVATE glad)

endif ()
