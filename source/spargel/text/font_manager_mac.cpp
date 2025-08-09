#include "spargel/text/font_manager_mac.h"

#include "spargel/base/logging.h"
#include "spargel/base/unique_ptr.h"
#include "spargel/text/font_mac.h"

namespace spargel::text {
    base::UniquePtr<FontManager> FontManager::create() {
        return base::makeUnique<FontManagerMac>();
    }
    FontMac* FontManagerMac::translateFont(CTFontRef font) {
        CoreTextFont key{font};
        auto result = fonts_.get(key);
        if (result) {
            return *result;
        }
        spargel_log_info("allocated new font for CoreText font %p", (void*)font);
        auto new_entry = new FontMac(font);
        fonts_.set(key, new_entry);
        return new_entry;
    }
    FontMac* FontManagerMac::defaultFont() {
        if (default_font_) {
            return default_font_;
        }
        auto font = CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, 0, nullptr);
        default_font_ = translateFont(font);
        return default_font_;
    }
    Font* FontManagerMac::matchDescriptor([[maybe_unused]] FontDescriptor const& descriptor) {
        // TODO
        return defaultFont();
    }
}  // namespace spargel::text
