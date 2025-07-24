#include "spargel/base/platform.h"

/* libc */
#include <stdio.h>

using namespace spargel::base;

int main() {
    auto path = get_executable_path();
    printf("Current executable path: \"%s\" \n", CString(path).data());
    printf("Path length: %zu\n", path.length());
    return 0;
}
