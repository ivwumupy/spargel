#include <spargel/config.h>
#include <spargel/ui/platform.h>

namespace spargel::ui {

#if SPARGEL_IS_LINUX
    base::unique_ptr<Platform> make_platform_xcb();
#elif SPARGEL_IS_MACOS
    base::unique_ptr<Platform> makePlatformAppKit();
#elif SPARGEL_IS_WINDOWS
    base::unique_ptr<Platform> make_platform_win32();
#elif SPARGEL_UI_DUMMY
    base::unique_ptr<Platform> makePlatformDummy();
#endif

    base::unique_ptr<Platform> makePlatform() {
#if SPARGEL_IS_LINUX
        return make_platform_xcb();
#elif SPARGEL_IS_MACOS
        return makePlatformAppKit();
#elif SPARGEL_IS_WINDOWS
        return make_platform_win32();
#elif SPARGEL_UI_DUMMY
        return makePlatformDummy();
#else
        return nullptr;
#endif
    }

}
