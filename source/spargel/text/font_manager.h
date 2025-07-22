#pragma once

#include "spargel/base/unique_ptr.h"
#include "spargel/base/vector.h"
#include "spargel/text/font.h"

namespace spargel::text {
    class FontManager {
    public:
        static base::UniquePtr<FontManager> create();

        virtual ~FontManager() = default;

        template <typename T, typename... Args>
        Font* create(Args&&... args) {
            auto u = base::makeUnique<T>(base::forward<Args>(args)...);
            auto p = u.get();
            fonts_.push(base::move(u));
            return p;
        }

    private:
        base::Vector<base::UniquePtr<Font>> fonts_;
    };
}  // namespace spargel::text
