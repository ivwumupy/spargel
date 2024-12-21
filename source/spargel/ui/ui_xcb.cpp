
#include <spargel/base/base.h>
#include <spargel/base/logging.h>
#include <spargel/ui/ui_xcb.h>

/* for clock_gettime */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

/* libc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* xcb */
#include <xcb/xcb.h>

namespace spargel::ui {

    base::unique_ptr<platform> make_platform_xcb() { return base::make_unique<platform_xcb>(); }

    platform_xcb::platform_xcb() : platform(platform_kind::xcb) {
        /*
         * Open the connection to the X server.
         * Use the DISPLAY environment variable as the default display name.
         */
        _connection = xcb_connect(NULL, NULL);

        /* Get the first screen */
        _screen = xcb_setup_roots_iterator(xcb_get_setup(_connection)).data;

        spargel_log_debug("Information of XCB screen %d:", _screen->root);
        spargel_log_debug("  width:\t%d", _screen->width_in_pixels);
        spargel_log_debug("  height:\t%d", _screen->height_in_pixels);
        spargel_log_debug("  white pixel:\t%08x", _screen->white_pixel);
        spargel_log_debug("  black pixel:\t%08x", _screen->black_pixel);

        xcb_intern_atom_cookie_t cookie;
        xcb_intern_atom_reply_t* reply;

        cookie = _intern_atom_cookie(1, "WM_PROTOCOLS");
        reply = _intern_atom_reply(cookie);
        _atom_wm_protocols = reply->atom;
        free(reply);

        cookie = _intern_atom_cookie(0, "WM_DELETE_WINDOW");
        reply = _intern_atom_reply(cookie);
        _atom_wm_delete_window = reply->atom;
        free(reply);
    }

    platform_xcb::~platform_xcb() { xcb_disconnect(_connection); }

    const float FPS = 60;
    const unsigned int SECOND_NS = 1000000000U;
    const unsigned int FRAME_DELTA_NS = (unsigned int)(SECOND_NS / FPS);

    static bool in_delta(struct timespec* t1, struct timespec* t2, unsigned int delta) {
        if (t1->tv_sec == t2->tv_sec) {
            return t2->tv_nsec < t1->tv_nsec + delta;
        } else if (t1->tv_sec == t2->tv_sec - 1) {
            return t2->tv_nsec + SECOND_NS < t1->tv_nsec + delta;
        } else {
            return false;
        }
    }

    void platform_xcb::start_loop() {
        xcb_generic_event_t* event;
        struct timespec t1, t2, duration = {.tv_sec = 0, .tv_nsec = 0};
        while (true) {
            bool should_stop = true;
            for (auto& window : _windows) {
                if (!window->_closed) {
                    should_stop = false;
                    break;
                }
            }
            if (should_stop) break;

            event = xcb_poll_for_event(_connection);
            if (!event) {
                clock_gettime(CLOCK_MONOTONIC, &t1);

                _run_render_callbacks();
                xcb_flush(_connection);

                /*
                 * wait for the next frame
                 * Normally the sleeping only occurs once each frame.
                 */
                while (true) {
                    clock_gettime(CLOCK_MONOTONIC, &t2);
                    if (in_delta(&t1, &t2, FRAME_DELTA_NS)) {
                        duration.tv_nsec = FRAME_DELTA_NS;
                        clock_nanosleep(CLOCK_MONOTONIC, 0, &duration, NULL);
                    } else {
                        break;
                    }
                }

                continue;
            }

            switch (event->response_type & 0x7f) {
            case XCB_CLIENT_MESSAGE: {
                auto* client_message_event = (xcb_client_message_event_t*)event;
                if (client_message_event->data.data32[0] == _atom_wm_delete_window) {
                    for (auto& window : _windows) {
                        if (window->_id == client_message_event->window) {
                            window->_closed = true;
                            window->delegate()->on_close_requested();
                            window->delegate()->on_closed();
                            break;
                        }
                    }
                }
            } break;
            case XCB_EXPOSE: {
                _run_render_callbacks();
                xcb_flush(_connection);
            } break;
            case XCB_KEY_PRESS: {
                // TODO
            } break;
            default:
                break;
            }

            free(event);
        }
    }

    void platform_xcb::_run_render_callbacks() {
        for (auto& window : _windows) {
            auto delegate = window->delegate();
            if (!delegate) {
                spargel_log_fatal("window delegate not set");
                spargel_panic_here();
            }
            delegate->on_render();
        }
    }

    xcb_intern_atom_cookie_t platform_xcb::_intern_atom_cookie(u8 only_if_exists,
                                                               const char* name) {
        return xcb_intern_atom(_connection, only_if_exists, strlen(name), name);
    }

    xcb_intern_atom_reply_t* platform_xcb::_intern_atom_reply(xcb_intern_atom_cookie_t cookie) {
        return xcb_intern_atom_reply(_connection, cookie, nullptr);
    }

    base::unique_ptr<window> platform_xcb::make_window(u32 width, u32 height) {
        xcb_window_t id = xcb_generate_id(_connection);

        u32 mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
        u32 values[2] = {_screen->white_pixel, XCB_EVENT_MASK_EXPOSURE};

        xcb_create_window(_connection, XCB_COPY_FROM_PARENT, /* depth*/
                          id, _screen->root,                 /* parent window */
                          0, 0,                              /* x, y */
                          width, height,                     /* width, height */
                          10,                                /* border width */
                          XCB_WINDOW_CLASS_INPUT_OUTPUT,     /* class */
                          _screen->root_visual,              /* visual */
                          mask, values);

        xcb_change_property(_connection, XCB_PROP_MODE_REPLACE, id, _atom_wm_protocols,
                            XCB_ATOM_ATOM, XCB_ATOM_VISUALID, 1, &_atom_wm_delete_window);

        xcb_map_window(_connection, id);
        xcb_flush(_connection);

        return base::make_unique<window_xcb>(this, id);
    }

    window_xcb::window_xcb(platform_xcb* platform, xcb_window_t id)
        : _platform(platform), _id(id), _closed(false) {
        platform->_windows.push(this);
    }

    window_xcb::~window_xcb() {}

    void window_xcb::set_title(char const* title) {
        xcb_change_property(_platform->_connection, XCB_PROP_MODE_REPLACE, _id, XCB_ATOM_WM_NAME,
                            XCB_ATOM_STRING, 8, strlen(title), title);
    }

    window_handle window_xcb::handle() {
        window_handle handle{};
        handle.xcb.connection = _platform->_connection;
        handle.xcb.window = _id;
        return handle;
    }

}  // namespace spargel::ui
