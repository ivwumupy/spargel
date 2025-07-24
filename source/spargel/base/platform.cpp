#include "spargel/base/platform.h"

#include "spargel/base/const.h"

namespace spargel::base {

    // FIXME
    String get_executable_path() {
        char* buf = (char*)base::default_allocator()->allocate(PATH_MAX);
        usize len = _get_executable_path(buf, PATH_MAX);
        if (len >= PATH_MAX) {
            buf = (char*)base::default_allocator()->resize(buf, PATH_MAX, len + 1);
            _get_executable_path(buf, len + 1);
        }
        buf[len] = '\0';
        String s = string_from_range(buf, buf + len);
        base::default_allocator()->free(buf, PATH_MAX);
        return s;
    }

}  // namespace spargel::base
