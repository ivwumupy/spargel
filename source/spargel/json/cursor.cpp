#include "spargel/json/cursor.h"

/* libc */
#include <string.h>

namespace spargel::json {

    bool Cursor::tryEatChar(char ch) {
        if (cur + 1 > end) return false;
        if (peek() != ch) return false;
        advance();
        return true;
    }

    bool Cursor::tryEatString(const char* str) {
        usize len = (usize)strlen(str);
        return tryEatBytes((const u8*)str, len);
    }

    bool Cursor::tryEatBytes(const u8* bytes, usize len) {
        if (cur + len > end) return false;
        if (memcmp(bytes, cur, len) != 0) return false;
        cur += len;
        return true;
    }

}  // namespace spargel::json
