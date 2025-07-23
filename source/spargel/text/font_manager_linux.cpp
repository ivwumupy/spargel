#include "spargel/text/font_manager_linux.h"

#include "spargel/text/font_linux.h"

using namespace spargel::base::literals;

namespace spargel::text {

    FontManagerLinux::FontManagerLinux() : resource_manager(""_sv) {
#if SPARGEL_ENABLE_FREETYPE
        FT_Init_FreeType(&ft_library);
#endif
    }

    FontManagerLinux::~FontManagerLinux() {
#if SPARGEL_ENABLE_FREETYPE
        FT_Done_FreeType(ft_library);
#endif
    }

    base::UniquePtr<Font> FontManagerLinux::createDefaultFont() {
        auto default_font_filepath = "/usr/share/fonts/truetype/msttcorefonts/Times_New_Roman.ttf";

        if (!default_font_file.hasValue()) {
            auto resource = resource_manager.open(resource::ResourceId(default_font_filepath));
            if (!resource.hasValue()) {
                spargel_log_fatal("Unable to open default font: %s.", default_font_filepath);
                spargel_panic_here();
            }
            default_font_file = base::move(resource);
        }

        spargel_check(default_font_file.hasValue());

        return base::makeUnique<FontLinux>(this, default_font_file.value()->getSpan(),
                                           12 /* default font size is 12pt */);
    }

    base::UniquePtr<FontManager> FontManager::create() {
        return base::makeUnique<FontManagerLinux>();
    }

}  // namespace spargel::text
