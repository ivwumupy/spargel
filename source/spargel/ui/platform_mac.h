#pragma once

#include <spargel/base/unique_ptr.h>
#include <spargel/ui/platform.h>

//
#import <AppKit/AppKit.h>

@interface SpargelApplicationDelegate : NSObject <NSApplicationDelegate>
@end

namespace spargel::ui {
    class PlatformAppKit final : public Platform {
    public:
        PlatformAppKit();
        ~PlatformAppKit() override;

        void startLoop() override;

        base::UniquePtr<Window> makeWindow(u32 width, u32 height) override;
        base::UniquePtr<TextSystem> createTextSystem() override;

    private:
        void initGlobalMenu();

        NSApplication* _app;
    };

    base::UniquePtr<Platform> makePlatformAppKit();

}  // namespace spargel::ui
