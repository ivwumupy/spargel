#include <CoreText/CoreText.h>

int main() {
    // auto font = CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, 0, nullptr);
    // CFShow(font);
    // auto traits = CTFontCopyTraits(font);
    // CFShow(traits);
    // auto symbolic = CTFontGetSymbolicTraits(font);
    // auto stylistic = symbolic & kCTFontTraitClassMask;
    // printf("%u\n", stylistic);
    auto family_name = CFStringCreateWithCString(nullptr, "Times", kCFStringEncodingUTF8);
    void const* keys[] = {kCTFontFamilyNameAttribute};
    void const* values[] = {family_name};
    auto dict = CFDictionaryCreate(kCFAllocatorDefault, keys, values, 1, NULL, NULL);
    CFShow(dict);
    auto font_desc = CTFontDescriptorCreateWithAttributes(dict);
    CFShow(font_desc);
    auto font = CTFontCreateWithFontDescriptor(font_desc, 0, nullptr);
    CFShow(font);
    return 0;
}
