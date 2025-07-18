#pragma once

#include <spargel/ui/platform.h>
#include <spargel/ui/text_system.h>
#include <spargel/ui/window.h>

namespace spargel::ui {

    class WindowDummy final : public Window {
    public:
        void setTitle(char const* title) override {}

        void setAnimating(bool) override {}
        void requestRedraw() override {}

        WindowHandle getHandle() override { return {}; }

        void bindRenderer(render::UIRenderer*) override {}
        void setTextFocus(bool focus) override {}
    };

    class TextSystemDummy final : public TextSystem {
    public:
        LineLayout layoutLine(base::StringView str) override;
        RasterResult rasterizeGlyph(GlyphId id, void*) override;
    };

    class PlatformDummy final : public Platform {
    public:
        PlatformDummy() : Platform(PlatformKind::dummy) {}

        void startLoop() override {}

        base::unique_ptr<Window> makeWindow(u32 width, u32 height) override {
            return base::make_unique<WindowDummy>();
        }
        base::unique_ptr<TextSystem> createTextSystem() override {
            return base::make_unique<TextSystemDummy>();
        }
    };

    base::unique_ptr<Platform> makePlatformDummy();

}  // namespace spargel::ui
