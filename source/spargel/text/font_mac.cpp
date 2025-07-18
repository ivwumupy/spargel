#include <spargel/base/checked_convert.h>
#include <spargel/base/logging.h>
#include <spargel/base/string.h>
#include <spargel/text/font_mac.h>

//
#include <math.h>

//
#import <CoreGraphics/CoreGraphics.h>

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
    Bitmap FontMac::rasterGlyph(GlyphId id, float scale) {
        auto glyph_info = glyphInfo(id);
        auto rect = glyph_info.bounding_box;

        u32 width = (u32)ceil(rect.size.width * scale);
        u32 height = (u32)ceil(rect.size.height * scale);

        if (width == 0 || height == 0) {
            spargel_log_info("zero sized glyph!");
            return {};
        }

        // for anti-aliasing
        // width += 4;
        // height += 4;

        Bitmap bitmap;
        bitmap.width = width;
        bitmap.height = height;
        bitmap.data.reserve(width *
                            height);  // only alpha. 8 bits per channel and 1 bytes per pixel
        bitmap.data.set_count(width * height);
        // memset(bitmap.data.data(), 0x3f, width * height);
        memset(bitmap.data.data(), 0x00, width * height);

        auto color_space = CGColorSpaceCreateDeviceGray();
        auto ctx = CGBitmapContextCreate(bitmap.data.data(), width, height,
                                         8,      // bits per channel
                                         width,  // bytes per row
                                         color_space, kCGImageAlphaOnly);
        CGContextTranslateCTM(ctx, -rect.origin.x * scale, -rect.origin.y * scale);
        // scale does not change the translate part!!!
        CGContextScaleCTM(ctx, scale, scale);

        // CGContextSetShouldAntialias(ctx, true);

        // shift for anti-aliasing
        CGPoint point = CGPointMake(0, 0);
        // CGPoint point = CGPointMake(1, 1);

        CGGlyph glyph = base::checkedConvert<CGGlyph>(id.value);
        CTFontDrawGlyphs(object_, &glyph, &point, 1, ctx);

        CFRelease(color_space);
        CGContextRelease(ctx);

        // Bitmap result;
        // result.bitmap = base::move(bitmap);
        // result.glyph_width = rect.size.width * scale;
        // result.glyph_height = rect.size.height * scale;
        // result.descent = rect.origin.y * scale;

        return bitmap;
    }
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
    base::UniquePtr<Font> createDefaultFont() {
        return base::makeUnique<FontMac>(
            CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, 50, nullptr));
    }
}  // namespace spargel::text
