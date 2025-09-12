#pragma once

#include "spargel/ui/window.h"

#include <Windows.h>

namespace spargel::ui {
    class PlatformWin32;

    class WindowWin32 : public Window {
    public:
        WindowWin32(PlatformWin32* platform, HWND hwnd);
        ~WindowWin32() override;

        void setTitle(char const* title) override;

        void setAnimating(bool animating) override { animating_ = animating; }
        bool animating() const { return animating_; }

        void requestRedraw() override {}

        WindowHandle getHandle() override;

        void bindRenderer(render::UIRenderer* renderer) override {}

        void setTextFocus(bool focus) override {}

        float scaleFactor() override { return 1.0; }

        float width() override;
        float height() override;

    private:
        PlatformWin32* platform_;
        HWND hwnd_;
        bool animating_ = false;
    };
}