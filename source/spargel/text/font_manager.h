#pragma once

#include "spargel/base/unique_ptr.h"
#include "spargel/text/font.h"

namespace spargel::text {
    class FontManager {
    public:
        static base::UniquePtr<FontManager> create();

        virtual ~FontManager() = default;

        virtual base::UniquePtr<Font> createDefaultFont() = 0;
    };
}  // namespace spargel::text
