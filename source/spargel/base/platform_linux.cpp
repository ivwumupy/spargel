
#include <spargel/base/platform.h>

/* libc */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* GNU */
#include <execinfo.h>

namespace spargel::base {

    usize _get_executable_path(char* buf, usize buf_size) {
        return readlink("/proc/self/exe", buf, buf_size);
    }

    void PrintBacktrace() {
        void* buffer[256];
        int nptrs = backtrace(buffer, 128);

        char** strings = backtrace_symbols(buffer, nptrs);
        if (!strings) {
            fputs("error calling backtrace_symbols", stderr);
            return;
        }

        for (int i = 0; i < nptrs; i++) {
            fputs("  ", stderr);
            fputs(strings[i], stderr);
            fputc('\n', stderr);
        }

        free(strings);
    }

}  // namespace spargel::base
