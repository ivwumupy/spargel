#pragma once

#include "spargel/base/hash_map.h"
#include "spargel/base/string.h"
#include "spargel/base/unique_ptr.h"
#include "spargel/ui/platform.h"
#include "spargel/ui/window.h"

// platform
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>

@interface SpargelHostView : NSView <NSTextInputClient>
@end

@interface SpargelWindowDelegate : NSObject <NSWindowDelegate>
@end

namespace spargel::ui {

    class TextInputDelegate {
    public:
        virtual ~TextInputDelegate() = default;

        virtual bool hasMarkedText() { return false; }
        virtual void setMarkedText([[maybe_unused]] id string,
                                   [[maybe_unused]] NSRange selected,
                                   [[maybe_unused]] NSRange replaced) {}
        virtual NSRange getMarkedRange() { return NSMakeRange(NSNotFound, 0); }
        virtual void unmarkText() {}
        virtual NSArray<NSAttributedStringKey>* validAttributesForMarkedText() {
            return nil;
        }
        virtual NSRange getSelectedRange() {
            return NSMakeRange(NSNotFound, 0);
        }
        virtual void insertText([[maybe_unused]] id string,
                                [[maybe_unused]] NSRange replaced) {}
        virtual NSAttributedString* attributedSubstringForProposedRange(
            [[maybe_unused]] NSRange range,
            [[maybe_unused]] NSRangePointer actual) {
            return nil;
        }
        virtual NSUInteger characterIndexForPoint(
            [[maybe_unused]] NSPoint point) {
            return NSNotFound;
        }
        virtual NSRect firstRectForCharacterRange(
            [[maybe_unused]] NSRange range,
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

        void emitMouseMoved(MouseMovedEvent const& e);

        void _updateSize(float width, float height) {
            width_ = width;
            height_ = height;
        }

        void _enable_text_cursor();

        void setTextDelegate(TextInputDelegate* delegate) {
            _text_delegate = delegate;
        }
        TextInputDelegate* getTextDelegate() { return _text_delegate; }

        auto ns_view() { return ns_view_; }
        auto ns_window() { return ns_window_; }

        void bindRenderer(render::UIRenderer* renderer) override;

        void setTextFocus(bool focus) override { text_focus_ = focus; }
        bool text_focus() const { return text_focus_; }

        float scaleFactor() override;

        NSWindow* getNSWindow() { return ns_window_; }

        float width() override;
        float height() override;

        WindowDelegate* delegate() { return getDelegate(); }

        CAMetalLayer* metalLayer() { return metal_layer_; }

    private:
        NSWindow* ns_window_;
        SpargelWindowDelegate* ns_window_delegate_;
        SpargelHostView* ns_view_;
        CAMetalLayer* metal_layer_;

        float width_;
        float height_;

        float drawable_width_;
        float drawable_height_;

        bool animating_ = true;

        // TODO:
        [[maybe_unused]] NSTextInsertionIndicator* _text_cursor;

        TextInputDelegate* _text_delegate = nullptr;

        bool text_focus_ = false;
    };

}  // namespace spargel::ui
