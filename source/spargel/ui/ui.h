#pragma once

#include <spargel/base/span.h>
#include <spargel/base/string_view.h>
#include <spargel/base/unique_ptr.h>
#include <spargel/base/vector.h>

namespace spargel::ui {

    enum class platform_kind {
        android,
        appkit,
        dummy,
        uikit,
        wayland,
        win32,
        xcb,
    };

    /// the native handle of a window
    struct window_handle {
        union {
            struct {
                void* window;
            } android;
            struct {
                void* layer;
            } apple;
            struct {
                void* display;
                void* surface;
            } wayland;
            struct {
                void* hinstance;
                void* hwnd;
            } win32;
            struct {
                void* connection;
                int window;
            } xcb;
        };
    };

    enum keyboard_action {
        press,
        release,
    };

    enum physical_key {
        escape,
        key_a,
        key_b,
        key_c,
        key_d,
        key_e,
        key_f,
        key_g,
        key_h,
        key_i,
        key_j,
        key_k,
        key_l,
        key_m,
        key_n,
        key_o,
        key_p,
        key_q,
        key_r,
        key_s,
        key_t,
        key_u,
        key_v,
        key_w,
        key_x,
        key_y,
        key_z,
        space,
        key_delete,

        unknown,
    };

    struct keyboard_event {
        keyboard_action action;
        physical_key key;

        // todo: hack
        char toChar();
    };

    class window_delegate {
    public:
        virtual ~window_delegate() = default;

        /// @brief the window is requested to draw the contents
        virtual void on_render() {}

        /// @brief the window is requested to close
        virtual void on_close_requested() {}

        /// @brief the window is closed
        virtual void on_closed() {}

        /// @brief a keyboard event is received
        virtual void on_keyboard(keyboard_event& e) {}
    };

    class window {
    public:
        virtual ~window() = default;

        void set_delegate(window_delegate* delegate) { _delegate = delegate; }
        window_delegate* delegate() { return _delegate; }

        virtual void set_title(char const* title) = 0;

        virtual void setAnimating(bool animating) = 0;

        virtual void requestRedraw() = 0;

        // todo: improve
        virtual window_handle handle() = 0;

    private:
        window_delegate* _delegate = nullptr;
    };

    struct Bitmap {
        usize width;
        usize height;
        base::vector<u8> data;
    };

    using GlyphId = u32;
    struct GlyphPosition {
        float x;
        float y;
    };

    struct LayoutRun {
        // TODO: font id
        base::vector<GlyphId> glyphs;
        base::vector<GlyphPosition> positions;
        float width;
    };

    struct LineLayout {
        base::vector<LayoutRun> runs;
    };

    struct RasterResult {
        Bitmap bitmap;
        float glyph_width;
        float glyph_height;
        float descent;
    };

    class TextSystem {
    public:
        virtual ~TextSystem() = default;

        virtual LineLayout layoutLine(base::string_view str) = 0;
        /// warning: alpha only
        virtual RasterResult rasterizeGlyph(GlyphId id) = 0;
    };

    class platform {
    public:
        virtual ~platform() = default;

        platform_kind kind() const { return _kind; }

        virtual void start_loop() = 0;

        virtual base::unique_ptr<window> make_window(u32 width, u32 height) = 0;

        virtual base::unique_ptr<TextSystem> createTextSystem() = 0;

    protected:
        explicit platform(platform_kind k) : _kind{k} {}

    private:
        platform_kind _kind;
    };

    base::unique_ptr<platform> make_platform();

    class DrawCmd {};

    class View;

    /// A LayoutManager is owned by a View. It manages how the View's children should be laid out
    /// within the View's content bounds.
    ///
    class LayoutManager {
    public:
        virtual ~LayoutManager() = default;

        /// layout the children of |view|
        virtual void layout(View* view) = 0;
    };

    class View {
    public:
        View();
        virtual ~View();

        View* parent() { return _parent; }
        base::span<View*> children();

        void addChild(View* v) {
            _children.push(v);
            v->setParent(this);
        }
        void setParent(View* v) { _parent = v; }

        void setLayoutManager();
        void getLayoutManager();

        void layout();

        /// compute the preferred size of the view given constraint
        void getPreferredSize();

    private:
        View* _parent = nullptr;
        base::vector<View*> _children;
        base::unique_ptr<LayoutManager> _layout_manager;
    };

}  // namespace spargel::ui
