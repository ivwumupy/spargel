
#include <spargel/base/platform.h>

// Emscripten
#include <emscripten.h>
#include <emscripten/console.h>

// libc
#include <string.h>

namespace spargel::base {

    usize _get_executable_path(char* buf, usize buf_size) {
        char const todo[] = "TODO";
        strcpy(buf, todo);
        return strlen(todo);
    }

    // FIXME
    void PrintBacktrace() { emscripten_console_trace(""); }

}  // namespace spargel::base
