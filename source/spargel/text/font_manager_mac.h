#pragma once

#include "spargel/base/bit_cast.h"
#include "spargel/base/hash.h"
#include "spargel/base/hash_map.h"
#include "spargel/base/tag_invoke.h"
#include "spargel/text/font_mac.h"
#include "spargel/text/font_manager.h"

//
#include <CoreText/CoreText.h>

namespace spargel::text {
    struct CoreTextFont {
        CTFontRef object;
        friend bool operator==(CoreTextFont lhs, CoreTextFont rhs);
        friend void tag_invoke(base::tag<base::hash>, base::HashRun& run, CoreTextFont font);
    };

    class FontManagerMac final : public FontManager {
    public:
        FontMac* translateFont(CTFontRef font);

    private:
        // Warp the handle.
        base::HashMap<CoreTextFont, FontMac*> fonts_;
    };
}
