#include <spargel/config.h>
#include <spargel/ui/platform.h>

namespace spargel::ui {

#if SPARGEL_IS_LINUX
    base::unique_ptr<Platform> makePlatformXcb();
#elif SPARGEL_IS_MACOS
    base::unique_ptr<Platform> makePlatformAppKit();
#elif SPARGEL_IS_WINDOWS
    base::unique_ptr<Platform> makePlatformWin32();
#elif SPARGEL_UI_DUMMY
    base::unique_ptr<Platform> makePlatformDummy();
#endif

    base::unique_ptr<Platform> makePlatform() {
#if SPARGEL_IS_LINUX
        return makePlatformXcb();
#elif SPARGEL_IS_MACOS
        return makePlatformAppKit();
#elif SPARGEL_IS_WINDOWS
        return makePlatformWin32();
#elif SPARGEL_UI_DUMMY
        return makePlatformDummy();
#else
        return nullptr;
#endif
    }

}
