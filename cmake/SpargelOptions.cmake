#### Spargel Options ####

if(SPARGEL_IS_EMSCRIPTEN)
    set(SPARGEL_ENABLE_OPENGL_DEFAULT OFF)
    set(SPARGEL_FILE_MMAP_DEFAULT OFF)
    set(SPARGEL_GPU_ENABLE_VULKAN_DEFAULT OFF)
else()
    set(SPARGEL_ENABLE_OPENGL_DEFAULT ON)
    set(SPARGEL_FILE_MMAP_DEFAULT ON)
    set(SPARGEL_GPU_ENABLE_VULKAN_DEFAULT ON)
endif()

if(SPARGEL_IS_WINDOWS)
    set(SPARGEL_GPU_ENABLE_DIRECTX_DEFAULT ON)
else()
    set(SPARGEL_GPU_ENABLE_DIRECTX_DEFAULT OFF)
endif()

if(SPARGEL_IS_MACOS)
    set(SPARGEL_ENABLE_METAL_DEFAULT ON)
else()
    set(SPARGEL_ENABLE_METAL_DEFAULT OFF)
endif()

spargel_add_option(SPARGEL_ENABLE_ADDRESSS_ANITIZER "enable address sanitizer" ON)
spargel_add_option(SPARGEL_ENABLE_ASSERT "enable assert()" ON)
spargel_add_option(SPARGEL_ENABLE_COVERAGE "enable coverge" OFF)
spargel_add_option(SPARGEL_ENABLE_LOG_ANSI_COLOR "enable ANSI color for logging" OFF)
spargel_add_option(SPARGEL_ENABLE_OPENGL "enable OpenGL backend" ${SPARGEL_ENABLE_OPENGL_DEFAULT})
spargel_add_option(SPARGEL_ENABLE_METAL "enable metal backend" ${SPARGEL_ENABLE_METAL_DEFAULT})
spargel_add_option(SPARGEL_ENABLE_THIRD_PARTY "enable third-party libraries" OFF)

spargel_add_option(SPARGEL_FILE_MMAP "enable mmap for file reading" ${SPARGEL_FILE_MMAP_DEFAULT})
spargel_add_option(SPARGEL_GIT_USE_SSH "use ssh for git remote operations" OFF)
spargel_add_option(SPARGEL_GPU_ENABLE_DIRECTX "enable directx backend" ${SPARGEL_GPU_ENABLE_DIRECTX_DEFAULT})
spargel_add_option(SPARGEL_GPU_ENABLE_VULKAN "enable vulkan backend" ${SPARGEL_GPU_ENABLE_VULKAN_DEFAULT})

spargel_add_option(SPARGEL_TRACE_ALLOCATION "trace allocation" ON)

#### Platform Specific Options ####

if (SPARGEL_IS_ANDROID)
    set(SPARGEL_ANDROID_TARGET simple_entry_demo CACHE STRING "the target to be built" )
    message(STATUS "Android build target: ${SPARGEL_ANDROID_TARGET}")
endif ()

if (SPARGEL_IS_WINDOWS)
    set(SPARGEL_VS_BUILD_TYPE Debug CACHE STRING "Visual Studio build type")
    message(STATUS "Visual Studio build type: ${SPARGEL_VS_BUILD_TYPE}")
endif ()
