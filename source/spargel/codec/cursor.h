#pragma once

#include <spargel/base/types.h>

namespace spargel::codec {

    struct cursor {
        char const* cur;
        char const* end;
    };

    struct Cursor {
        const char* cur;
        const char* end;
    };

    static inline bool cursorIsEnd(const Cursor& cursor) { return cursor.cur >= cursor.end; }

    static inline u8 cursorPeek(const Cursor& cursor) {
        if (cursorIsEnd(cursor)) return 0;
        return *cursor.cur;
    }

    static inline void cursorAdvance(Cursor& cursor, int steps = 1) { cursor.cur += steps; }

    static inline u8 cursorGetChar(Cursor& cursor) {
        u8 ch = cursorPeek(cursor);
        cursorAdvance(cursor);
        return ch;
    }

    /*
     * These funtions will move the cursor if succeeds,
     * otherwise the cursor will not move.
     */
    bool cursorTryEatChar(Cursor& cursor, char ch);
    bool cursorTryEatBytes(Cursor& cursor, const u8* bytes, usize count);
    bool cursorTryEatString(Cursor& cursor, const char* str);

}  // namespace spargel::codec
