#pragma once

#include "spargel/base/optional.h"
#include "spargel/resource/directory.h"
#include "spargel/text/font_manager.h"

#if SPARGEL_ENABLE_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

namespace spargel::text {

    class FontManagerLinux : public FontManager {
    public:
        FontManagerLinux();

        ~FontManagerLinux() override;

        base::UniquePtr<Font> createDefaultFont() override;

        Font* defaultFont() override { return nullptr; }

        Font* matchDescriptor(FontDescriptor const& descriptor) override { return nullptr; }

    private:
        friend class FontLinux;

        resource::ResourceManagerDirectory resource_manager;
        base::Optional<base::UniquePtr<resource::Resource>> default_font_file;

#if SPARGEL_ENABLE_FREETYPE
        FT_Library ft_library;
#endif
    };

}  // namespace spargel::text
