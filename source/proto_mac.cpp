#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>

int main() {
    auto modes = CGDisplayCopyAllDisplayModes(CGMainDisplayID(), nullptr);
    CFArrayApplyFunction(
        modes, CFRangeMake(0, CFArrayGetCount(modes)),
        [](const void* value, void*) {
            auto mode = (CGDisplayModeRef)value;
            auto width = CGDisplayModeGetWidth(mode);
            auto height = CGDisplayModeGetHeight(mode);
            printf("mode: %zu x %zu\n", width, height);
        },
        nullptr);
    return 0;

    CFDictionaryRef trait_dict = [] {
        void const* keys[] = {kCTFontSymbolicTrait};
        auto n = kCTFontClassOldStyleSerifs;
        // auto n = kCTFontClassSansSerif;
        auto number = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &n);
        void const* values[] = {number};
        return CFDictionaryCreate(kCFAllocatorDefault, keys, values, 1, NULL,
                                  NULL);
    }();
    CFShow(trait_dict);

    CFArrayRef langs = [] {
        auto name = CFSTR("zh");
        void const* values[] = {name};
        return CFArrayCreate(kCFAllocatorDefault, values, 1, nullptr);
    }();

    // auto family_name = CFStringCreateWithCString(nullptr, "Times",
    // kCFStringEncodingUTF8); void const* keys[] =
    // {kCTFontFamilyNameAttribute}; void const* values[] = {family_name};
    void const* keys[] = {kCTFontTraitsAttribute, kCTFontLanguagesAttribute};
    void const* values[] = {trait_dict, langs};
    auto dict =
        CFDictionaryCreate(kCFAllocatorDefault, keys, values, 2, NULL, NULL);
    CFShow(dict);
    auto font_desc = CTFontDescriptorCreateWithAttributes(dict);
    CFShow(font_desc);
    auto font = CTFontCreateWithFontDescriptor(font_desc, 0, nullptr);
    CFShow(font);

    auto fonts =
        CTFontDescriptorCreateMatchingFontDescriptors(font_desc, nullptr);
    CFShow(fonts);

    auto traits = CTFontCopyTraits(font);
    CFShow(traits);
    auto symbolic = CTFontGetSymbolicTraits(font);
    auto stylistic = symbolic & kCTFontTraitClassMask;
    printf("%u\n", stylistic);
    return 0;
}
