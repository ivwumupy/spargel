#include "spargel/ui/ui_dummy.h"

namespace spargel::ui {

    base::unique_ptr<Platform> makePlatformDummy() {
        return base::make_unique<PlatformDummy>();
    }

}  // namespace spargel::ui
