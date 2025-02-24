
#include <spargel/base/const.h>
#include <spargel/util/path.h>

namespace spargel::util {

    base::string dirname(const base::string& path) {
        if (path.length() == 0) return base::string_from_cstr(".");

        const char* data = path.data();
        char const* cur = path.end() - 1;

        // Skip trailing separators
        while (cur >= path.begin() && *cur == PATH_SPLIT) {
            cur--;
        }

        // Handle case where all characters were separators
        if (cur < path.begin()) {
            if (path.length() > 0 && *path.begin() == PATH_SPLIT) {
                return base::string_from_cstr("/");
            } else {
                return base::string_from_cstr(".");
            }
        }

        // Find last separator
        for (; cur >= path.begin(); cur--) {
            if (*cur == PATH_SPLIT) break;
        }

        // If no separator found, return "."
        if (cur < path.begin()) return base::string_from_cstr(".");

        // If we're at root, return "/"
        if (cur == path.begin() && *cur == PATH_SPLIT) {
            return base::string_from_range(data, cur + 1);
        }

        // Return path up to but not including the separator
        return base::string_from_range(data, cur);
    }

}  // namespace spargel::util
