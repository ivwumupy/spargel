#pragma once

#include "spargel/base/types.h"

namespace spargel::json {

    struct Cursor {
        const char* cur;
        const char* end;

        bool isEnd() const { return cur >= end; }

        u8 peek() const {
            if (isEnd()) return 0;
            return (u8)*cur;
        }

        void advance(int steps = 1) { cur += steps; }

        u8 consumeChar() {
            u8 ch = peek();
            advance();
            return ch;
        }

        /*
         * These funtions will move the cursor if succeeds,
         * otherwise the cursor will not move.
         */
        bool tryEatChar(char ch);
        bool tryEatBytes(const u8* bytes, usize count);
        bool tryEatString(const char* str);
    };

}  // namespace spargel::json
