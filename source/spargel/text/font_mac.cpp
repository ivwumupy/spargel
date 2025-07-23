#include <spargel/base/checked_convert.h>
#include <spargel/base/logging.h>
#include <spargel/base/string.h>
#include <spargel/text/font_mac.h>

//
#include <math.h>

//
#include <CoreGraphics/CoreGraphics.h>

namespace spargel::text {
    namespace {
        CGGlyph toCGGlyph(GlyphId id) { return base::checkedConvert<CGGlyph>(id.value); }
    }  // namespace
    FontMac::FontMac(CTFontRef object) : object_{object} {
        CFRetain(object_);

        auto name = CTFontCopyFullName(object_);
        auto range = CFRangeMake(0, CFStringGetLength(name));
        CFIndex len;
        spargel_check(
            CFStringGetBytes(name, range, kCFStringEncodingUTF8, 0, false, nullptr, 0, &len) > 0);
        name_.resize(base::checkedConvert<usize>(len));
        spargel_check(CFStringGetBytes(name, range, kCFStringEncodingUTF8, 0, false,
                                       (u8*)name_.data(), (CFIndex)name_.length(), &len) == len);
        CFRelease(name);
    }
    FontMac::~FontMac() { CFRelease(object_); }
    Bitmap FontMac::rasterGlyph(GlyphId id, float scale, math::Vector2f subpixel_position) {
        auto glyph_info = glyphInfo(id);
        auto rect = glyph_info.bounding_box;

        u32 width = (u32)ceil(rect.size.width * scale);
        u32 height = (u32)ceil(rect.size.height * scale);

        if (width == 0 || height == 0) {
            spargel_log_info("zero sized glyph!");
            return {};
        }

        // spargel_log_info("font raster scale: %.3f", scale);

        // TODO: Check whether subpixel position is non-zero.
        width += 1;
        height += 1;

        Bitmap bitmap;
        bitmap.width = width;
        bitmap.height = height;
        bitmap.data.reserve(width *
                            height);  // only alpha. 8 bits per channel and 1 bytes per pixel
        bitmap.data.set_count(width * height);
        memset(bitmap.data.data(), 0x00, width * height);

        auto color_space = CGColorSpaceCreateDeviceGray();
        auto ctx = CGBitmapContextCreate(bitmap.data.data(), width, height,
                                         8,      // bits per channel
                                         width,  // bytes per row
                                         color_space, kCGImageAlphaOnly);
        // NOTE: Scale does not change the translate part.
        //
        // TODO: Read the docs. Why we have to translate before scale?
        CGContextTranslateCTM(ctx,
                              //(-rect.origin.x + subpixel_position.x) * scale,
                              //(-rect.origin.y + subpixel_position.y) * scale);
                              -rect.origin.x * scale + subpixel_position.x,
                              -rect.origin.y * scale + subpixel_position.y);
        CGContextScaleCTM(ctx, scale, scale);

        CGContextSetAllowsFontSubpixelPositioning(ctx, true);
        CGContextSetAllowsFontSubpixelQuantization(ctx, true);
        CGContextSetShouldSubpixelPositionFonts(ctx, true);
        CGContextSetShouldSubpixelQuantizeFonts(ctx, true);

        // TODO: Shift for anti-aliasing.
        // TODO: Subpixel shifting is done here or above.
        CGPoint point = CGPointMake(0, 0);
        // CGPoint point = CGPointMake(1, 1);

        CGGlyph glyph = base::checkedConvert<CGGlyph>(id.value);
        CTFontDrawGlyphs(object_, &glyph, &point, 1, ctx);

        CFRelease(color_space);
        CGContextRelease(ctx);

        return bitmap;
    }
    Bitmap FontMac::rasterGlyph(GlyphId id, float scale) {
        return rasterGlyph(id, scale, math::Vector2f{0.0f, 0.0f});
    }
    // CoreText Coodinate
    // ------------------
    // The origin is at bottom-left and the y-axis goes up.
    //
    GlyphInfo FontMac::glyphInfo(GlyphId id) {
        CGGlyph glyph = toCGGlyph(id);

        CGRect rect;
        CTFontGetBoundingRectsForGlyphs(object_, kCTFontOrientationHorizontal, &glyph, &rect, 1);

        CGSize advance;
        CTFontGetAdvancesForGlyphs(object_, kCTFontOrientationHorizontal, &glyph, &advance, 1);

        GlyphInfo info;
        info.bounding_box.origin.x = (float)rect.origin.x;
        info.bounding_box.origin.y = (float)rect.origin.y;
        info.bounding_box.size.width = (float)rect.size.width;
        info.bounding_box.size.height = (float)rect.size.height;
        info.horizontal_advance = (float)advance.width;

        return info;
    }
    base::UniquePtr<Font> createDefaultFont() {
        return base::makeUnique<FontMac>(
            CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, 0, nullptr));
    }
}  // namespace spargel::text
