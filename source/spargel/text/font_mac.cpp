#include <spargel/base/checked_convert.h>
#include <spargel/text/font_mac.h>

namespace spargel::text {
    namespace {
        CGGlyph toCGGlyph(GlyphId id) { return base::checkedConvert<CGGlyph>(id.value); }
    }  // namespace
    FontMac::FontMac(CTFontRef object) : object_{object} {
        auto name = CTFontCopyFullName(object_);
        auto range = CFRangeMake(0, CFStringGetLength(name));
        CFIndex len;
        spargel_check(
            CFStringGetBytes(name, range, kCFStringEncodingUTF8, 0, false, nullptr, 0, &len) > 0);
        name_.resize(base::checkedConvert<usize>(len));
        spargel_check(CFStringGetBytes(name, range, kCFStringEncodingUTF8, 0, false,
                                       (u8*)name_.data(), name_.length(), &len) == len);
        CFRelease(name);
    }
    FontMac::~FontMac() { CFRelease(object_); }
    Bitmap FontMac::rasterGlyph(GlyphId id) { spargel_panic_here(); }
    GlyphInfo FontMac::glyphInfo(GlyphId id) {
        CGGlyph glyph = toCGGlyph(id);

        CGRect rect;
        CTFontGetBoundingRectsForGlyphs(object_, kCTFontOrientationHorizontal, &glyph, &rect, 1);

        CGSize advance;
        CTFontGetAdvancesForGlyphs(object_, kCTFontOrientationHorizontal, &glyph, &advance, 1);

        GlyphInfo info;
        info.bounding_box.origin.x = rect.origin.x;
        info.bounding_box.origin.y = rect.origin.y;
        info.bounding_box.size.width = rect.size.width;
        info.bounding_box.size.height = rect.size.height;
        info.horizontal_advance = advance.width;

        return info;
    }
    Font* createDefaultFont() {
        return new FontMac(CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, 12, nullptr));
    }
}  // namespace spargel::text
