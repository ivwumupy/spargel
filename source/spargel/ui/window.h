#pragma once

#include "spargel/ui/event.h"

namespace spargel::render {
    class UIRenderer;
}

namespace spargel::ui {

    // the native handle of a window
    struct WindowHandle {
        union HandleUnion {
            struct AndroidHandle {
                void* window;
            } android;
            struct AppleHandle {
                void* layer;
            } apple;
            struct WaylandHandle {
                void* display;
                void* surface;
            } wayland;
            struct Win32Handle {
                void* hinstance;
                void* hwnd;
            } win32;
            struct XcbHandle {
                void* connection;
                int window;

                // Xlib things, for GLX
                void* display;
                void* visual_info;
            } xcb;
        } value;
    };

    class WindowDelegate {
    public:
        virtual ~WindowDelegate() = default;

        // The window is requested to draw the contents.
        virtual void onRender() {}

        // The window is requested to close.
        // TODO: willClose
        virtual void onCloseRequested() {}

        // The window is closed.
        // TODO: didClose
        virtual void onClosed() {}

        // A keyboard event is received.
        virtual void onKeyboard([[maybe_unused]] KeyboardEvent& e) {}

        // virtual void onKeyDown([[maybe_unused]] PhysicalKey key) {}

        [[deprecated]]
        virtual void onMouseDown([[maybe_unused]] float x,
                                 [[maybe_unused]] float y) {}
        virtual void onMouseDown([[maybe_unused]] MouseDownEvent const& e) {}

        // User has moved the mouse with left button pressed.
        virtual void onMouseDragged([[maybe_unused]] float dx,
                                    [[maybe_unused]] float dy) {}

        virtual void onMouseMoved([[maybe_unused]] const MouseMovedEvent& e) {}
    };

    // The coordinate system is TLO (top-left origin).
    //
    class Window {
    public:
        virtual ~Window() = default;

        void setDelegate(WindowDelegate* delegate) { _delegate = delegate; }
        WindowDelegate* getDelegate() { return _delegate; }

        virtual void setTitle(char const* title) = 0;

        virtual void setAnimating(bool animating) = 0;

        virtual void requestRedraw() = 0;

        // TODO: Improve.
        // This is an abstraction leak. One should obtain the handle from the
        // subclasses.
        [[deprecated]]
        virtual WindowHandle getHandle() = 0;

        virtual void bindRenderer(render::UIRenderer* renderer) = 0;

        // HACK: Whether this window has focus on text input.
        // TODO: Introduce TextInputClient.
        virtual void setTextFocus(bool focus) = 0;

        virtual float scaleFactor() = 0;

        virtual float width() = 0;
        virtual float height() = 0;

    private:
        WindowDelegate* _delegate = nullptr;
    };

}  // namespace spargel::ui
