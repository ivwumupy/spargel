#pragma once

#include "spargel/ui/platform.h"
#include "spargel/ui/window.h"

namespace spargel::ui {

    class WindowDummy final : public Window {
    public:
        void setTitle(char const* title) override {}

        void setAnimating(bool) override {}
        void requestRedraw() override {}

        WindowHandle getHandle() override { return {}; }

        void bindRenderer(render::UIRenderer*) override {}
        void setTextFocus(bool focus) override {}
        float scaleFactor() override { return 1.0; }

        float width() override { return 0.0f; }
        float height() override { return 0.0f; }
    };

    class PlatformDummy final : public Platform {
    public:
        PlatformDummy() : Platform(PlatformKind::dummy) {}

        void startLoop() override {}

        base::unique_ptr<Window> makeWindow(u32 width, u32 height) override {
            return base::make_unique<WindowDummy>();
        }
    };

    base::unique_ptr<Platform> makePlatformDummy();

}  // namespace spargel::ui
