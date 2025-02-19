#pragma once

#include <spargel/base/vector.h>
#include <spargel/config.h>
#include <spargel/ui/text_linux.h>
#include <spargel/ui/ui.h>

/* X11 */
#include <X11/Xlib-xcb.h>
#include <X11/Xlib.h>
#include <xcb/xcb.h>

#if SPARGEL_ENABLE_OPENGL
/* GLX */
#include <GL/glx.h>
#endif

namespace spargel::ui {

    class window_xcb;

    class platform_xcb : public platform {
        friend window_xcb;

    public:
        platform_xcb();
        ~platform_xcb() override;

        void start_loop() override;

        base::unique_ptr<window> make_window(u32 width, u32 height) override;

        base::unique_ptr<TextSystem> createTextSystem() override {
            return base::make_unique<TextSystemLinux>();
        }

    private:
        Display* _display;

        xcb_connection_t* _connection;
        xcb_screen_t* _screen;

#if SPARGEL_ENABLE_OPENGL
        XVisualInfo* _visual_info;
#endif

        base::vector<window_xcb*> _windows;

        void _run_render_callbacks();

        xcb_atom_t _atom_wm_protocols;
        xcb_atom_t _atom_wm_delete_window;

        xcb_intern_atom_cookie_t _intern_atom_cookie(u8 only_if_exists, const char* name);
        xcb_intern_atom_reply_t* _intern_atom_reply(xcb_intern_atom_cookie_t cookie);
    };

    base::unique_ptr<platform> make_platform_xcb();

    class window_xcb : public window {
        friend platform_xcb;

    public:
        window_xcb(platform_xcb* platform, xcb_window_t id);
        ~window_xcb() override;

        void set_title(char const* title) override;

        void setAnimating(bool animating) override {}

        void requestRedraw() override {}

        window_handle handle() override;

    private:
        platform_xcb* _platform;
        xcb_window_t _id;
        bool _closed;
    };

}  // namespace spargel::ui
