#include "spargel/ui/window_win32.h"

#include "spargel/base/check.h"
#include "spargel/ui/platform_win32.h"

namespace spargel::ui {

    WindowWin32::WindowWin32(PlatformWin32* platform, HWND hwnd)
        : platform_{platform}, hwnd_{hwnd} {}

    WindowWin32::~WindowWin32() { DestroyWindow(hwnd_); }

    void WindowWin32::setTitle(char const* title) {
        SetWindowTextA(hwnd_, title);
    }

    WindowHandle WindowWin32::getHandle() {
        WindowHandle handle{};
        handle.value.win32.hinstance = platform_->hInstance();
        handle.value.win32.hwnd = hwnd_;
        return handle;
    }

    float WindowWin32::width() {
        RECT rect;
        spargel_check(GetWindowRect(hwnd_, &rect));
        return rect.right - rect.left;
    }

    float WindowWin32::height() {
        RECT rect;
        spargel_check(GetWindowRect(hwnd_, &rect));
        return rect.bottom - rect.top;
    }

}  // namespace spargel::ui