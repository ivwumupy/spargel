#include "spargel/base/platform.h"

//
#include <stdio.h>

//
#include <Windows.h>

namespace spargel::base {

    struct dynamic_library_handle {};

    dynamic_library_handle* open_dynamic_library(const char* filename) {
        return (dynamic_library_handle*)LoadLibraryA(filename);
    }

    void close_dynamic_library(dynamic_library_handle* handle) { FreeLibrary((HMODULE)handle); }

    void* get_proc_address(dynamic_library_handle* handle, const char* name) {
        return (void*)GetProcAddress((HMODULE)handle, name);
    }

    usize _get_executable_path(char* buf, usize buf_size) {
        return GetModuleFileNameA(NULL, buf, buf_size);
    }

    // TODO: Symbolize the traces.
    void PrintBacktrace() { 
        void* entries[64];
        auto count = CaptureStackBackTrace(0, 64, entries, nullptr);
        for (int i = 0; i < count; i++) {
            printf("  # 0x%p <unknown>\n", entries[i]);
        }
    }

}  // namespace spargel::base
