#include "spargel/base/string_view.h"
#include "spargel/gpu/metal_context.h"
#include "spargel/render/ui_renderer_metal.h"
#include "spargel/render/ui_scene.h"
#include "spargel/text/text_shaper_mac.h"
#include "spargel/text/font_manager_mac.h"

//
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>

using namespace spargel;
using namespace spargel::base::literals;

struct GroundTruth {
    uint8_t* data;
    size_t width;
    size_t height;
    CGFloat ascent;
    CGFloat descent;
    CGFloat leading;
};

struct RenderResult {
    uint8_t* data;
};

inline constexpr auto TEXT = "hello,world测试日本語"_sv;

void writeToFile(char const* filename, uint8_t const* data, size_t w, size_t h) {
    auto file = fopen(filename, "w");

    fprintf(file, "P2\n%zu %zu\n255\n", w, h);

    for (size_t i = 0; i < w * h; i++) {
        fprintf(file, "%d ", data[i]);
    }

    fputc('\n', file);

    fclose(file);
}

void buildGroundTruth(GroundTruth& output) {
    auto str = CFStringCreateWithCString(nullptr, TEXT.data(), kCFStringEncodingUTF8);
    auto font = CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, 50, nullptr);
    void const* keys[] = {kCTFontAttributeName, kCTLigatureAttributeName};
    int val = 2;
    auto number = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &val);
    void const* values[] = {font, number};
    auto dict = CFDictionaryCreate(kCFAllocatorDefault, keys, values, 1, NULL, NULL);
    auto attr_str = CFAttributedStringCreate(kCFAllocatorDefault, str, dict);
    auto line = CTLineCreateWithAttributedString(attr_str);

    CGFloat ascent;
    CGFloat descent;
    CGFloat leading;
    auto width = CTLineGetTypographicBounds(line, &ascent, &descent, &leading);

    printf("%.3f %.3f %.3f %.3f\n", ascent, descent, leading, width);

    auto w = (size_t)ceil(width);
    auto h = (size_t)ceil(ascent + descent);

    auto data = (uint8_t*)malloc(w * h);
    memset(data, 0, w * h);

    auto color_space = CGColorSpaceCreateDeviceGray();
    auto ctx = CGBitmapContextCreate(data, w, h,
                                     8,  // bits per channel
                                     w,  // bytes per row
                                     color_space, kCGImageAlphaOnly);

    CGContextSetTextMatrix(ctx, CGAffineTransformIdentity);
    CGContextTranslateCTM(ctx, -leading, descent);
    CGContextScaleCTM(ctx, 1.0, 1.0);
    CGContextSetTextPosition(ctx, 0, 0);

    CGContextSetAllowsFontSubpixelPositioning(ctx, true);
    CGContextSetAllowsFontSubpixelQuantization(ctx, true);
    CGContextSetShouldSubpixelPositionFonts(ctx, true);
    CGContextSetShouldSubpixelQuantizeFonts(ctx, true);

    CTLineDraw(line, ctx);

    output.data = data;
    output.width = w;
    output.height = h;
    output.ascent = ascent;
    output.descent = descent;
    output.leading = leading;

    CFRelease(ctx);
    CFRelease(line);
    CFRelease(attr_str);
    CFRelease(dict);
    CFRelease(number);
    CFRelease(font);
    CFRelease(str);
}

void buildSpargel(GroundTruth const& truth, RenderResult& result) {
    auto context = new gpu::MetalContext;
    auto manager = new text::FontManagerMac;
    auto shaper = new text::TextShaperMac(manager);
    auto renderer = new render::UIRendererMetal(context, shaper);
    auto font = text::createDefaultFont();
    render::UIScene scene;
    scene.setRenderer(renderer);

    auto device = context->device();

    auto desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm_sRGB
                                                                   width:truth.width
                                                                  height:truth.height
                                                               mipmapped:false];
    desc.storageMode = MTLStorageModeShared;
    if (render::UIRendererMetal::use_compute) {
        desc.usage = MTLTextureUsageShaderWrite;
    } else {
        desc.usage = MTLTextureUsageRenderTarget;
    }
    auto texture = [device newTextureWithDescriptor:desc];

    scene.setClip(0, 0, (float)truth.width, (float)truth.height);
    scene.fillText(text::StyledText{TEXT, font.get()}, 0,
        (float)truth.height - (float)truth.descent, 0xFFFFFFFF);

    auto command_buffer = renderer->renderToTexture(scene, texture);

    auto buffer = [device newBufferWithLength:truth.width * truth.height * 4
                                      options:MTLResourceStorageModeShared];
    auto encoder = [command_buffer blitCommandEncoder];
    
    [encoder  copyFromTexture:texture
                  sourceSlice:0
                  sourceLevel:0
                 sourceOrigin:MTLOriginMake(0, 0, 0)
                   sourceSize:MTLSizeMake(truth.width, truth.height, 1)
                     toBuffer:buffer
            destinationOffset:0
       destinationBytesPerRow:truth.width * 4
     destinationBytesPerImage:truth.width * truth.height * 4];

    [encoder endEncoding];

    [command_buffer commit];
    [command_buffer waitUntilCompleted];

    result.data = (uint8_t*)malloc(truth.width * truth.height);
    for (usize i = 0; i < truth.width * truth.height; i++) {
        result.data[i] = ((uint8_t*)buffer.contents)[i * 4];
    }

    [texture release];
    [buffer release];
    delete renderer;
    delete shaper;
    delete context;
}

int abs(int x) {
    return x < 0 ? -x : x;
}

int main() {
    GroundTruth gt;
    RenderResult res;
    buildGroundTruth(gt);
    buildSpargel(gt, res);
    writeToFile("truth.pgm", gt.data, gt.width, gt.height);
    writeToFile("render.pgm", res.data, gt.width, gt.height);

    float s = 0.0f;
    for (size_t i = 0; i < gt.width * gt.height; i++) {
        auto d = (uint8_t)abs(res.data[i] - gt.data[i]);
        res.data[i] = d;
        s += (d / 255.0f) * (d / 255.0f);
    }
    writeToFile("diff.pgm", res.data, gt.width, gt.height);
    printf("l2 distance: %.3f\n", sqrtf(s / ((float)gt.width * (float)gt.height)));

    free(gt.data);
    free(res.data);
    return 0;
}
