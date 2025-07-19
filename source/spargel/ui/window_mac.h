#pragma once

#include <spargel/base/hash_map.h>
#include <spargel/base/string.h>
#include <spargel/base/unique_ptr.h>
#include <spargel/ui/platform.h>
#include <spargel/ui/text_system.h>
#include <spargel/ui/window.h>

// platform
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>

@interface SpargelMetalView : NSView <NSTextInputClient>  // temp hack
@end

@interface SpargelWindowDelegate : NSObject <NSWindowDelegate>
@end
namespace spargel::ui {

    class TextInputDelegate {
    public:
        virtual ~TextInputDelegate() = default;

        virtual bool hasMarkedText() { return false; }
        virtual void setMarkedText([[maybe_unused]] id string, [[maybe_unused]] NSRange selected,
                                   [[maybe_unused]] NSRange replaced) {}
        virtual NSRange getMarkedRange() { return NSMakeRange(NSNotFound, 0); }
        virtual void unmarkText() {}
        virtual NSArray<NSAttributedStringKey>* validAttributesForMarkedText() { return nil; }
        virtual NSRange getSelectedRange() { return NSMakeRange(NSNotFound, 0); }
        virtual void insertText([[maybe_unused]] id string, [[maybe_unused]] NSRange replaced) {}
        virtual NSAttributedString* attributedSubstringForProposedRange(
            [[maybe_unused]] NSRange range, [[maybe_unused]] NSRangePointer actual) {
            return nil;
        }
        virtual NSUInteger characterIndexForPoint([[maybe_unused]] NSPoint point) {
            return NSNotFound;
        }
        virtual NSRect firstRectForCharacterRange([[maybe_unused]] NSRange range,
                                                  [[maybe_unused]] NSRangePointer actual) {
            return NSMakeRect(0, 0, 0, 0);
        }
    };

    class WindowAppKit final : public Window {
    public:
        WindowAppKit(u32 width, u32 height);
        ~WindowAppKit() override;

        void setTitle(char const* title) override;

        void setAnimating(bool animating) override;
        void requestRedraw() override;

        WindowHandle getHandle() override;

        // todo: remove
        void _setDrawableSize(float width, float height);

        // todo: refactor
        void _bridge_render();

        void _bridge_key_down(int key, NSEventType type);

        void _bridgeMouseDown(float x, float y);
        void _bridgeMouseDragged(float dx, float dy);

        void _updateSize(float width, float height) {
            _width = width;
            _height = height;
        }

        void _enable_text_cursor();

        void setTextDelegate(TextInputDelegate* delegate) { _text_delegate = delegate; }
        TextInputDelegate* getTextDelegate() { return _text_delegate; }

        auto ns_view() { return _view; }
        auto ns_window() { return _window; }

        void bindRenderer(render::UIRenderer* renderer) override;

        void setTextFocus(bool focus) override { text_focus_ = focus; }
        bool text_focus() const { return text_focus_; }

        float scaleFactor() override;

    private:
        NSWindow* _window;
        SpargelWindowDelegate* _bridge;
        SpargelMetalView* _view;
        CAMetalLayer* _layer;

        float _width;
        float _height;

        float _drawable_width;
        float _drawable_height;

        bool _animating = true;

        [[maybe_unused]] NSTextInsertionIndicator* _text_cursor;

        TextInputDelegate* _text_delegate = nullptr;

        bool text_focus_ = false;
    };

    // class TextSystemAppKit final : public TextSystem {
    // public:
    //     TextSystemAppKit();
    //     ~TextSystemAppKit();

    //     LineLayout layoutLine(base::StringView str) override;
    //     RasterResult rasterizeGlyph(GlyphId id, void* font) override;

    // private:
    //     CTFontRef lookupFont(base::String const& name);

    //     CTFontRef _font;
    // };

}  // namespace spargel::ui
