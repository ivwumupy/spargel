#pragma once

#include <spargel/base/unique_ptr.h>
#include <spargel/ui/ui.h>

// platform
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>

@interface SpargelApplicationDelegate : NSObject <NSApplicationDelegate>
@end

@interface SpargelMetalView : NSView //<NSTextInputClient>  // temp hack
@end

@interface SpargelWindowDelegate : NSObject <NSWindowDelegate>
@end

namespace spargel::ui {

    class platform_appkit final : public platform {
    public:
        platform_appkit();
        ~platform_appkit() override;

        void start_loop() override;

        base::unique_ptr<window> make_window(u32 width, u32 height) override;
        base::unique_ptr<TextSystem> createTextSystem() override;

    private:
        void init_global_menu();

        NSApplication* _app;
    };

    base::unique_ptr<platform> make_platform_appkit();

    class TextInputDelegate {
    public:
        virtual bool hasMarkedText() { return false; }
        virtual void setMarkedText(id string, NSRange selected, NSRange replaced) {}
        virtual NSRange getMarkedRange() { return NSMakeRange(0, 0); }
        virtual void unmarkText() {}
        virtual NSArray<NSAttributedStringKey>* validAttributesForMarkedText() { return nil; }
        virtual NSRange getSelectedRange() { return NSMakeRange(0, 0); }
        virtual void insertText(id string, NSRange replaced) {}
        virtual NSAttributedString* attributedSubstringForProposedRange(NSRange range,
                                                                        NSRangePointer actual) {
            return nil;
        }
        virtual NSUInteger characterIndexForPoint(NSPoint point) { return 0; }
        virtual NSRect firstRectForCharacterRange(NSRange range, NSRangePointer actual) {
            return NSMakeRect(0, 0, 0, 0);
        }
    };

    class window_appkit final : public window {
    public:
        window_appkit(int width, int height);
        ~window_appkit() override;

        void set_title(char const* title) override;

        void setAnimating(bool animating) override;
        void requestRedraw() override;

        window_handle handle() override;

        // todo: remove
        void _set_drawable_size(float width, float height);

        // todo: refactor
        void _bridge_render();

        void _bridge_key_down(int key, NSEventType type);

        void _enable_text_cursor();

        void setTextDelegate(TextInputDelegate* delegate) { _text_delegate = delegate; }
        TextInputDelegate* getTextDelegate() { return _text_delegate; }

    private:
        NSWindow* _window;
        SpargelWindowDelegate* _bridge;
        SpargelMetalView* _view;
        CAMetalLayer* _layer;
        float _drawable_width;
        float _drawable_height;
        bool _animating = true;

        NSTextInsertionIndicator* _text_cursor;

        TextInputDelegate* _text_delegate = nullptr;
    };

    class TextSystemAppKit final : public TextSystem {
    public:
        TextSystemAppKit();
        ~TextSystemAppKit();

        LineLayout layoutLine(base::string_view str) override;
        RasterResult rasterizeGlyph(GlyphId id) override;

    private:
        CTFontRef _font;
    };

}  // namespace spargel::ui
