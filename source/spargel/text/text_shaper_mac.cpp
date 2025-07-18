#include <spargel/text/font_mac.h>
#include <spargel/text/font_manager.h>
#include <spargel/text/styled_text.h>
#include <spargel/text/text_shaper_mac.h>

namespace spargel::text {
    ShapedLine TextShaperMac::shapeLine(StyledText const& text) {
        auto font = static_cast<FontMac*>(text.font());

        // Convert to CFString
        auto cfstr = CFStringCreateWithBytes(kCFAllocatorDefault, (u8 const*)text.text().data(),
                                             text.text().length(), kCFStringEncodingUTF8, false);

        // Create CFAttributedString
        void const* keys[] = {kCTFontAttributeName, kCTLigatureAttributeName};
        int val = 2;
        auto number = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &val);
        void const* values[] = {font->object_, number};
        auto dict = CFDictionaryCreate(kCFAllocatorDefault, keys, values, 1, NULL, NULL);
        auto attr_str = CFAttributedStringCreate(kCFAllocatorDefault, cfstr, dict);

        // Shape using Core Text.
        auto line = CTLineCreateWithAttributedString(attr_str);

        auto glyph_runs = CTLineGetGlyphRuns(line);
        auto run_count = CFArrayGetCount(glyph_runs);

        base::vector<CGGlyph> glyphs;
        base::vector<CGPoint> points;

        ShapedLine result;
        ShapedSegment segment;

        for (CFIndex i = 0; i < run_count; i++) {
            CTRunRef run = (CTRunRef)CFArrayGetValueAtIndex(glyph_runs, i);
            auto count = CTRunGetGlyphCount(run);

            auto attr = CTRunGetAttributes(run);

            auto font = (CTFontRef)CFDictionaryGetValue(attr, kCTFontAttributeName);
            CFRetain(font);
            segment.font = FontManager::instance().create<FontMac>(font);

            glyphs.reserve(count);
            CTRunGetGlyphs(run, CFRangeMake(0, count), glyphs.data());
            glyphs.set_count(count);

            points.reserve(count);
            CTRunGetPositions(run, CFRangeMake(0, count), points.data());
            points.set_count(count);

            segment.glyphs.reserve(count);
            segment.glyphs.set_count(count);
            for (usize j = 0; j < count; j++) {
                segment.glyphs[j] = {glyphs[j]};
            }
            segment.positions.reserve(count);
            segment.positions.set_count(count);
            for (usize j = 0; j < count; j++) {
                segment.positions[j].x = points[j].x;
                segment.positions[j].y = points[j].y;
            }

            segment.width = CTRunGetTypographicBounds(run, CFRangeMake(0, count), NULL, NULL, NULL);

            result.segments.emplace(base::move(segment));
        }

        CFRelease(line);
        CFRelease(attr_str);
        CFRelease(number);
        CFRelease(cfstr);

        return result;
    }
    base::UniquePtr<TextShaper> TextShaper::create() { return base::makeUnique<TextShaperMac>(); }
}  // namespace spargel::text
