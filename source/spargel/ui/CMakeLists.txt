spargel_add_library(
    NAME ui
    PRIVATE
        event.cpp
        platform.cpp
        ui_dummy.cpp
        view.cpp
        view_host.cpp
    PRIVATE_ANDROID
        ui_android.cpp
    PRIVATE_LINUX
    PRIVATE_MACOS
        platform_mac.mm
        window_mac.mm
    PRIVATE_WINDOWS
        ui_win32.cpp
    DEPS
        base
        render
)

spargel_add_executable(
    NAME demo_ui
    PRIVATE
        demo_ui.cpp
    DEPS
        base
        gpu
        render
        text
        ui
)

if (SPARGEL_IS_ANDROID)
    get_target_property(ANDROID_GAME_ACTIVITY_INCLUDE game-activity::game-activity INTERFACE_INCLUDE_DIRECTORIES)
    target_include_directories(ui PRIVATE ${ANDROID_GAME_ACTIVITY_INCLUDE})
endif ()

if (SPARGEL_IS_LINUX)
    if (SPARGEL_LINUX_IS_DESKTOP)

        target_sources(ui PRIVATE ui_xcb.cpp xkb_util.cpp)

        target_link_libraries(ui PRIVATE
            X11 xcb X11-xcb
            ${XKBCOMMON_LIBRARIES} ${XKBCOMMON_X11_LIBRARIES})

        if (SPARGEL_ENABLE_OPENGL)
            target_link_libraries(ui PRIVATE OpenGL::GLX)
        endif ()

    endif ()
endif ()

if (APPLE)
    target_link_libraries(ui
        PRIVATE
            ${FRAMEWORK_APP_KIT} ${FRAMEWORK_QUARTZ_CORE} ${FRAMEWORK_GAME_CONTROLLER}
    )
endif ()

#spargel_add_executable(
#    NAME window_demo
#    PRIVATE window_demo.cpp
#    DEPS ui
#)
#
#spargel_add_application(
#    NAME window_demo2
#    PRIVATE window_demo2.cpp
#    DEPS entry
#)
