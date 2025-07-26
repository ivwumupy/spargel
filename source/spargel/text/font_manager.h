#pragma once

#include "spargel/base/unique_ptr.h"
#include "spargel/text/font.h"
#include "spargel/text/font_descriptor.h"

namespace spargel::text {
    class FontManager {
    public:
        static base::UniquePtr<FontManager> create();

        virtual ~FontManager() = default;

        virtual base::UniquePtr<Font> createDefaultFont() = 0;

        virtual Font* defaultFont() = 0;

        virtual Font* matchDescriptor(FontDescriptor const& descriptor) = 0;
    };
}  // namespace spargel::text
