#include <spargel/text/font.h>
#include <spargel/base/string.h>

//
#include <stdio.h>

using namespace spargel;

int main() {
    auto font = text::createDefaultFont();
    printf("default font: %s\n", base::CString{font->name()}.data());
    return 0;
}
