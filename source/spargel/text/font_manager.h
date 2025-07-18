#pragma once

namespace spargel::text {
    class Font;
    class FontManager {
    public:
        virtual ~FontManager() = default;
        virtual Font* createDefaultFont(float size);
    };
}  // namespace spargel::text
