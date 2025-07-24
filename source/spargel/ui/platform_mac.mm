#include "spargel/ui/platform_mac.h"
#include "spargel/base/check.h"
#include "spargel/ui/window_mac.h"

@implementation SpargelApplicationDelegate
- (bool)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender {
    return true;
}
@end

namespace spargel::ui {
    base::unique_ptr<Platform> makePlatformAppKit() { return base::make_unique<PlatformAppKit>(); }
    PlatformAppKit::PlatformAppKit() : Platform(PlatformKind::appkit) {
        _app = [NSApplication sharedApplication];
        [_app setActivationPolicy:NSApplicationActivationPolicyRegular];

        SpargelApplicationDelegate* delegate = [[SpargelApplicationDelegate alloc] init];
        // NSApp.delegate is a weak reference
        _app.delegate = delegate;

        initGlobalMenu();
    }
    PlatformAppKit::~PlatformAppKit() {}
    void PlatformAppKit::startLoop() { [_app run]; }
    base::unique_ptr<Window> PlatformAppKit::makeWindow(u32 width, u32 height) {
        spargel_check(width > 0 && height > 0);
        return base::make_unique<WindowAppKit>(width, height);
    }
    void PlatformAppKit::initGlobalMenu() {
        NSMenu* menu_bar = [[NSMenu alloc] init];

        NSMenu* app_menu = [[NSMenu alloc] initWithTitle:@"Spargel"];
        [app_menu addItemWithTitle:@"About Spargel" action:nil keyEquivalent:@""];
        [app_menu addItemWithTitle:@"Check for Updates" action:nil keyEquivalent:@""];
        [app_menu addItemWithTitle:@"Quit Spargel" action:@selector(terminate:) keyEquivalent:@"q"];

        NSMenuItem* app_menu_item = [[NSMenuItem alloc] init];
        [app_menu_item setSubmenu:app_menu];
        [menu_bar addItem:app_menu_item];

        // mainMenu is a strong reference
        _app.mainMenu = menu_bar;
    }
}  // namespace spargel::ui
