#pragma once

#include <spargel/ui/ui.h>

namespace spargel::ui {

    class window_dummy final : public window {
    public:
        void set_title(char const* title) override {}

        void setAnimating(bool) override {}
        void requestRedraw() override {}

        window_handle handle() override { return {}; }
    };

    class platform_dummy final : public platform {
    public:
        platform_dummy() : platform(platform_kind::dummy) {}

        void start_loop() override {}

        base::unique_ptr<window> make_window(u32 width, u32 height) override {
            return base::make_unique<window_dummy>();
        }
        base::unique_ptr<TextSystem> createTextSystem() override { return nullptr; }
    };

    base::unique_ptr<platform> make_platform_dummy();

}  // namespace spargel::ui
