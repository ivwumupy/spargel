#include "spargel/text/font_manager_mac.h"

#include "spargel/base/logging.h"
#include "spargel/base/unique_ptr.h"
#include "spargel/text/font_mac.h"

//
#include <CoreFoundation/CoreFoundation.h>

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
    base::UniquePtr<Font> FontManagerMac::createDefaultFont() {
        return base::makeUnique<FontMac>(
            CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, 50, nullptr));
    }
    FontMac* FontManagerMac::defaultFont() {
        if (default_font_) {
            return default_font_;
        }
        auto font = CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, 50, nullptr);
        default_font_ = translateFont(font);
        return default_font_;
    }
    Font* FontManagerMac::matchDescriptor([[maybe_unused]] FontDescriptor const& descriptor) {
        // TODO
        return defaultFont();
    }
    bool operator==(CoreTextFont lhs, CoreTextFont rhs) { return CFEqual(lhs.object, rhs.object); }
    void tag_invoke(base::tag<base::hash>, base::HashRun& run, CoreTextFont font) {
        run.combine(base::bitCast<CFHashCode, u64>(CFHash(font.object)));
    }
}  // namespace spargel::text
