#pragma once

#include <Windows.h>

#include "spargel/base/types.h"
#include "spargel/base/unique_ptr.h"
#include "spargel/base/vector.h"
#include "spargel/ui/platform.h"

namespace spargel::ui {

    class WindowWin32;

    class PlatformWin32 : public Platform {
    public:
        explicit PlatformWin32(HINSTANCE hInstance)
            : Platform(PlatformKind::win32), hInstance_{hInstance} {}

        void startLoop() override;

        base::UniquePtr<Window> makeWindow(u32 width, u32 height) override;

        HINSTANCE hInstance() const { return hInstance_; }

    private:
        HINSTANCE hInstance_;
        base::Vector<WindowWin32*> windows_;
    };

    base::UniquePtr<Platform> makePlatformWin32();

}  // namespace spargel::ui