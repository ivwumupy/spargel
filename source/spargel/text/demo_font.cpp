#include "spargel/base/string.h"
#include "spargel/text/font.h"
#include "spargel/text/font_manager.h"

//
#include <stdio.h>

using namespace spargel;

int main() {
    auto font_manager = text::FontManager::create();
    auto font = font_manager->defaultFont();
    printf("default font: %s\n", base::CString{font->name()}.data());
    return 0;
}
