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

@interface SpargelApplicationDelegate : NSObject <NSApplicationDelegate>
@end

@interface SpargelMetalView : NSView <NSTextInputClient>  // temp hack
@end

@interface SpargelWindowDelegate : NSObject <NSWindowDelegate>
@end

namespace spargel::ui {

    class PlatformAppKit final : public Platform {
    public:
        PlatformAppKit();
        ~PlatformAppKit() override;

        void startLoop() override;

        base::unique_ptr<Window> makeWindow(u32 width, u32 height) override;
        base::unique_ptr<TextSystem> createTextSystem() override;

    private:
        void initGlobalMenu();

        NSApplication* _app;
    };

    base::unique_ptr<Platform> makePlatformAppKit();

    class TextInputDelegate {
    public:
        virtual ~TextInputDelegate() = default;

        virtual bool hasMarkedText() { return false; }
        virtual void setMarkedText(id string, NSRange selected, NSRange replaced) {}
        virtual NSRange getMarkedRange() { return NSMakeRange(NSNotFound, 0); }
        virtual void unmarkText() {}
        virtual NSArray<NSAttributedStringKey>* validAttributesForMarkedText() { return nil; }
        virtual NSRange getSelectedRange() { return NSMakeRange(NSNotFound, 0); }
        virtual void insertText(id string, NSRange replaced) {}
        virtual NSAttributedString* attributedSubstringForProposedRange(NSRange range,
                                                                        NSRangePointer actual) {
            return nil;
        }
        virtual NSUInteger characterIndexForPoint(NSPoint point) { return NSNotFound; }
        virtual NSRect firstRectForCharacterRange(NSRange range, NSRangePointer actual) {
            return NSMakeRect(0, 0, 0, 0);
        }
    };

    class WindowAppKit final : public Window {
    public:
        WindowAppKit(int width, int height);
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
    };

    struct CTFontRAII {
        CTFontRef ref;
        ~CTFontRAII() {
            CFRelease(ref);
        }
    };

    class TextSystemAppKit final : public TextSystem {
    public:
        TextSystemAppKit();
        ~TextSystemAppKit();

        LineLayout layoutLine(base::StringView str) override;
        RasterResult rasterizeGlyph(GlyphId id, void* font) override;

    private:
        CTFontRef lookupFont(base::String const& name);

        CTFontRef _font;
        base::HashMap<base::String, CTFontRAII> fonts_;
    };

}  // namespace spargel::ui
