#include "spargel/base/check.h"
#include "spargel/base/logging.h"
#include "spargel/base/optional.h"
#include "spargel/base/string_view.h"
#include "spargel/base/unique_ptr.h"
#include "spargel/base/vector.h"
#include "spargel/gpu/gpu.h"
#include "spargel/render/atlas_packer.h"
#include "spargel/resource/directory.h"
#include "spargel/resource/resource.h"
#include "spargel/ui/platform.h"
#include "spargel/ui/window.h"
#include "spargel/ui/window_mac.h"

#define USE_VULKAN 1
#if SPARGEL_IS_MACOS
#define USE_METAL 1
#else
#define USE_METAL 0
#endif

// #if USE_METAL
// #include "spargel/gpu/demo/shader.metallib.inc"
// #endif

// #if USE_VULKAN
// #include "spargel/gpu/demo/fragment_shader.spirv.inc"
// #include "spargel/gpu/demo/vertex_shader.spirv.inc"
// #endif

#include <stdio.h>

#import <GameController/GameController.h>

namespace spargel::gpu {
    namespace {
        using namespace base::literals;

        struct vertex_shader_info {
            static constexpr VertexBufferLocation position_buffer = {
                .apple = {0},
                .vulkan = {0},
            };
            static constexpr VertexBufferLocation quad_buffer = {
                .apple = {1},
                .vulkan = {1},
            };
            static constexpr VertexBufferLocation uniform_buffer = {
                .apple = {2},
                .vulkan = {2},
            };
        };

        struct VertexPosition {
            float x;
            float y;
        };
        struct VertexColor {
            float r;
            float g;
            float b;
            float a;
        };

        static constexpr VertexPosition positions[] = {
            {0, 0}, {1, 0}, {1, 1}, {1, 1}, {0, 1}, {0, 0},
        };

        struct UniformData {
            struct {
                float width;
                float height;
            } viewport;
        };

        struct float2 {
            float x;
            float y;
        };

        struct alignas(8) QuadData {
            float2 origin;
            float2 size;
            float2 cell_origin;
            u32 color;
        };

        // u32 max(u32 a, u32 b) { return a > b ? a : b; }

        class Renderer final : public ui::WindowDelegate, public ui::TextInputDelegate {
        public:
            Renderer(ui::WindowAppKit* window, resource::ResourceManager* resource_manager,
                     ui::TextSystem* text_system)
                : _window{window}, _manager{resource_manager}, _text_system{text_system} {
                _window->setDelegate(this);
                _window->setTextDelegate(this);
                _window->setTextFocus(true);

                // keyboard_ = [GCKeyboard coalescedKeyboard];
                // spargel_check(keyboard_);
                // keyboard_input_ = [keyboard_ keyboardInput];
                // spargel_check(keyboard_input_);
                // key_ = [keyboard_input_ buttonForKeyCode:GCKeyCodeKeyA];
                // spargel_check(key_);
                // key_.pressedChangedHandler = ^(GCControllerButtonInput*, float, bool pressed) {
                //   printf("GC Callback Key A: %s\n", pressed ? "pressed" : "released");
                // };

                _device = makeDevice(DeviceKind::metal);

                _surface = _device->createSurface(_window);

                view_ = _window->ns_view();
                input_context_ = view_.inputContext;
                // spargel_check(input_context_);

#if USE_METAL
                auto blob = _manager->open(resource::ResourceId("shader.metallib"_sv));

                _shader = _device->createShaderLibrary(blob.value()->getSpan());

                // TODO: Close the resource.
                // blob->close();
#endif

                RenderPipelineBuilder builder;
                builder.setVertexShader(_shader, "vertex_shader");
                builder.setFragmentShader(_shader, "fragment_shader");
                builder.addVertexBuffer(sizeof(VertexPosition));
                builder.addVertexAttribute(VertexAttributeFormat::float32x2, 0, 0, 0);
                builder.addColorAttachment(TextureFormat::bgra8_unorm, true);
                _pipeline = builder.build(_device.get());

                _positions = _device->createBuffer(
                    BufferUsage::vertex, base::make_span<u8>(sizeof(positions), (u8*)positions));

                _uniform_data.viewport.width = _surface->width();
                _uniform_data.viewport.height = _surface->height();
                _uniforms = _device->createBuffer(
                    BufferUsage::uniform,
                    base::make_span(sizeof(_uniform_data), (u8*)&_uniform_data));

                _queue = _device->createCommandQueue();

                _atlas = _device->createTexture(TEXTURE_SIZE, TEXTURE_SIZE);

                // _str.push('t');
                //

                // Arabia
                // _str.push(0xd9);
                // _str.push(0x84);
                // _str.push(0xd8);
                // _str.push(0xa7);

                // Arabia
                // _str.push(0xd9);
                // _str.push(0x85);
                // _str.push(0xd8);
                // _str.push(0xaa);
                // _str.push(0xd8);
                // _str.push(0xb3);
                // _str.push(0xd8);
                // _str.push(0xa7);
                // _str.push(0xd9);
                // _str.push(0x88);
                // _str.push(0xd9);
                // _str.push(0x8a);
                // _str.push(0xd9);
                // _str.push(0x86);

                // _str.push(0xe2);
                // _str.push(0x9d);
                // _str.push(0xa4);
                // _str.push(0xef);
                // _str.push(0xb8);
                // _str.push(0x8f);

                // _str.push(0xf0);
                // _str.push(0x9f);
                // _str.push(0x98);
                // _str.push(0x80);

                // _str.push(0xf0);
                // _str.push(0x9f);
                // _str.push(0x96);
                // _str.push(0x90);
                // _str.push(0xef);
                // _str.push(0xb8);
                // _str.push(0x8f);
                //
                //
                // _str.push(0xe4);
                // _str.push(0xbd);
                // _str.push(0xa0);
                // _str.push(0xe5);
                // _str.push(0xa5);
                // _str.push(0xbd);

                _window->setAnimating(_animating);
                _window->requestRedraw();
            }

            ~Renderer() {
                _device->destroyBuffer(_positions);
                _device->destroyRenderPipeline(_pipeline);
                _device->destroyShaderLibrary(_shader);
            }

            // Return: width of the text
            float drawText(base::StringView s, u32 color, float offset,
                           base::Vector<QuadData>& quads) {
                auto layout_result = _text_system->layoutLine(s);
                float width = 0;
                for (usize r = 0; r < layout_result.runs.count(); r++) {
                    auto& run = layout_result.runs[r];
                    for (usize i = 0; i < run.glyphs.count(); i++) {
                        auto& info = findOrInsert(run.glyphs[i], run.font);
                        QuadData quad;
                        quad.origin.x = offset + run.positions[i].x;
                        quad.origin.y = info.descent + run.positions[i].y;
                        // quad.size.width = info.width - 1;
                        quad.size.x = info.width - 1;
                        quad.size.y = info.height;
                        quad.cell_origin.x = info.x + 1;
                        quad.cell_origin.y = info.y + 1;
                        quad.color = color;
                        quads.emplace(quad);
                    }
                    width += run.width;
                }
                return width;
            }

            void onRender() override {
                // spargel_log_info("rendering: %s", base::CString{_str.begin(),
                // _str.end()}.data());

                auto layout_result =
                    _text_system->layoutLine(base::StringView(_str.begin(), _str.end()));

                _vquads.clear();
                text_width_ = 0;
                text_width_ += drawText(base::StringView{_str.begin(), _str.end()}, 0xff0000ff,
                                        text_width_, _vquads);
                anchor_offset_ = text_width_;
                text_width_ += drawText(base::StringView{marked_.begin(), marked_.end()},
                                        0xffff0000, text_width_, _vquads);
                // todo: use uniform
                float half_width = text_width_ / 2;
                for (usize i = 0; i < _vquads.count(); i++) {
                    _vquads[i].origin.x -= half_width;
                }
                if (_vquads.empty()) {
                    return;
                }
                _quads = _device->createBuffer(
                    BufferUsage::storage,
                    base::make_span(_vquads.count() * sizeof(QuadData), (u8*)_vquads.data()));

                auto texture = _surface->nextTexture();

                auto cmdbuf = _queue->createCommandBuffer();

                auto encoder = cmdbuf->beginRenderPass({
                    .color_attachments = {{
                        .texture = texture,
                        .clear_color = {0.0, 0.0, 0.0, 1.0},
                    }},
                });

                encoder->setViewport({0, 0, _surface->width(), _surface->height(), 0, 1});
                encoder->setRenderPipeline(_pipeline);
                encoder->setVertexBuffer(_positions, vertex_shader_info::position_buffer);
                encoder->setVertexBuffer(_uniforms, vertex_shader_info::uniform_buffer);
                encoder->setVertexBuffer(_quads, vertex_shader_info::quad_buffer);
                encoder->setTexture(_atlas);
                encoder->draw(0, 6, 0, _vquads.count());

                cmdbuf->endRenderPass(encoder);

                cmdbuf->present(_surface);

                cmdbuf->submit();

                _queue->destroyCommandBuffer(cmdbuf);
                _device->destroyBuffer(_quads);

                spargel_log_info("glyph cache count: %lu", _glyph_cache.count());
                // for (auto const& x : _glyph_cache) {
                //     printf("%u ", x.id);
                // }
                // printf("\n");
                //

                printf("Selection (%s) ", base::CString{_str.begin(), _str.end()}.data());
                selection_.dump();
                // printf("GC Key A: %s\n", key_.isPressed ? "pressed" : "released");
            }

            void deleteChar() {
                if (_str.empty()) {
                    return;
                }
                auto p = _str.end() - 1;
                if ((*p & 0b10000000) == 0) {
                    _str.pop();
                    return;
                }
                while ((*p & 0b11000000) == 0b10000000) {
                    _str.pop();
                    printf("%d\n", *p);
                    p--;
                }
                printf("%d\n", *p);
                spargel_check((*p & 0b11000000) == 0b11000000);
                _str.pop();
                p--;
            }

            void onKeyboard(ui::KeyboardEvent& e) override {
                if (e.key == ui::PhysicalKey::key_delete && e.action == ui::KeyboardAction::press) {
                    if (!selection_.marked_range.hasValue()) {
                        deleteChar();
                        selection_.head -= 1;
                    }
                    if (!_animating) {
                        _window->requestRedraw();
                    }
                } else if (e.key == ui::PhysicalKey::space &&
                           e.action == ui::KeyboardAction::press) {
                    // spargel_log_info("space pressed");
                    // _animating = !_animating;
                    // _window->setAnimating(_animating);
                } else {
                    printf("onKey: %c\n", e.toChar());
                    if (!_animating) {
                        _window->requestRedraw();
                    }
                }
                [[NSTextInputContext currentInputContext] invalidateCharacterCoordinates];
            }

            // TextInputDelegate
            bool hasMarkedText() override {
                bool result = selection_.marked_range.hasValue();
                printf("hasMarkedText -> %s\n", result ? "true" : "false");
                return result;
            }
            // TODO
            void setMarkedText(id string, NSRange selected, NSRange replaced) override {
                auto s = (NSAttributedString*)string;
                printf("setMarkedText \"%s\" selected=(%lu,%lu) replaced=(%lu,%lu)\n",
                       [[s string] UTF8String], static_cast<unsigned long>(selected.location),
                       static_cast<unsigned long>(selected.length),
                       static_cast<unsigned long>(replaced.location),
                       static_cast<unsigned long>(replaced.length));
                auto replace_range = selection_.replacement(replaced);
                auto length = [s length];
                if (length == 0) {
                    printf("deleting\n");
                    // todo: delete
                    unmarkText();
                } else {
                    selection_.marked_range =
                        base::makeOptional<NSRange>(NSMakeRange(replace_range.location, length));
                    // todo: replace
                    auto len = [[s string] lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
                    auto data = [[s string] UTF8String];
                    marked_.clear();
                    for (usize i = 0; i < len; i++) {
                        marked_.push(data[i]);
                    }
                }
                selection_.head = replace_range.location + selected.location;
                selection_.anchor = selection_.head + selected.length;
            }
            NSRange getMarkedRange() override {
                printf("getMarkedRange\n");
                if (!selection_.marked_range.hasValue()) {
                    return NSMakeRange(NSNotFound, 0);
                }
                return selection_.marked_range.value();
            }
            void unmarkText() override {
                printf("unmarkText\n");
                selection_.marked_range = base::nullopt;
                marked_.clear();
            }
            // TODO
            NSArray<NSAttributedStringKey>* validAttributesForMarkedText() override { return @[]; }
            NSRange getSelectedRange() override {
                printf("getSelectedRange\n");
                return selection_.range();
            }
            void insertText(id string, NSRange replaced) override {
                auto s = (NSString*)string;
                auto data = [s UTF8String];
                printf("insertText: \"%s\" (%lu,%lu)\n", data,
                       static_cast<unsigned long>(replaced.location),
                       static_cast<unsigned long>(replaced.length));
                auto len = [s lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
                for (usize i = 0; i < len; i++) {
                    _str.push(data[i]);
                }
                auto real_replaced = selection_.replacement(replaced);
                real_replaced.length = [s length];
                selection_ = Selection{(int)NSMaxRange(real_replaced)};
                unmarkText();
                _window->requestRedraw();
            }
            // TODO
            NSRect firstRectForCharacterRange(NSRange range, NSRangePointer actual) override {
                printf("firstRectForCharacterRange: range = (%lu, %lu)",
                       static_cast<unsigned long>(range.location),
                       static_cast<unsigned long>(range.length));
                if (actual != nullptr) {
                    printf(" actual = (%lu, %lu)", (unsigned long)(actual->location),
                           (unsigned long)(actual->length));
                }
                putchar('\n');
                auto w = _window->ns_window();
                auto b = NSMakeRect(500 - text_width_ / 2 + anchor_offset_, 500 - 0,
                                    text_width_ - anchor_offset_, 100);
                auto wr = [w convertRectFromBacking:b];
                return [w convertRectToScreen:wr];
            }
            // TODO
            NSAttributedString* attributedSubstringForProposedRange(
                NSRange range, NSRangePointer actual) override {
                printf("attributedSubstringForProposedRange\n");
                return nil;
            }
            // TODO
            NSUInteger characterIndexForPoint(NSPoint point) override {
                printf("characterIndexForPoint\n");
                return 0;
            }

        private:
            struct GlyphEntry {
                ui::GlyphId id;
                u32 x;
                u32 y;
                u32 width;
                u32 height;
                float glyph_width;
                float glyph_height;
                float descent;
                void* font;
            };

            struct Selection {
                int head = 0;
                int anchor = 0;
                base::Optional<NSRange> marked_range;

                Selection(int h) : head{h}, anchor{h}, marked_range{base::nullopt} {}

                void dump() {
                    printf("{ head = %d; anchor = %d; marked_range = ", head, anchor);
                    if (marked_range.hasValue()) {
                        printf("(%lu,%lu) }\n",
                               static_cast<unsigned long>(marked_range.value().location),
                               static_cast<unsigned long>(marked_range.value().length));
                    } else {
                        printf("none }\n");
                    }
                }
                NSRange range() {
                    if (head < anchor) {
                        return NSMakeRange(head, anchor - head);
                    } else {
                        return NSMakeRange(anchor, head - anchor);
                    }
                }
                NSRange replacement(NSRange proposed) {
                    if (proposed.location == NSNotFound) {
                        if (marked_range.hasValue()) {
                            return marked_range.value();
                        }
                        return range();
                    }
                    return proposed;
                }
            };

            static constexpr usize TEXTURE_SIZE = 2048;

            // todo: optimize
            GlyphEntry& findOrInsert(ui::GlyphId id, void* font) {
                printf("%p\n", font);
                for (usize i = 0; i < _glyph_cache.count(); i++) {
                    if (_glyph_cache[i].id == id && _glyph_cache[i].font == font) {
                        return _glyph_cache[i];
                    }
                }
                // not found
                auto raster = _text_system->rasterizeGlyph(id, font);
                auto& bitmap = raster.bitmap;

                // printf("bitmap %zu,%zu\n", bitmap.width, bitmap.height);

                auto pack_result = packer_.pack(bitmap.width, bitmap.height);
                if (!pack_result.hasValue()) {
                    spargel_log_fatal("glyph atlas out of space!");
                    spargel_panic_here();
                }
                // // try to insert
                // if (_cur_col + bitmap.width >= TEXTURE_SIZE) {
                //     // next row
                //     _cur_row = _nxt_row;
                //     _cur_col = 0;
                // }
                // u32 x = _cur_col;
                // _cur_col += bitmap.width;
                // _nxt_row = max(_nxt_row, _cur_row + bitmap.height);
                // if (_nxt_row >= TEXTURE_SIZE) {
                //     spargel_log_fatal("glyph atlas out of space!");
                //     spargel_panic_here();
                // }
                // u32 y = _cur_row;
                u32 x = pack_result.value().x;
                u32 y = pack_result.value().y;
                _glyph_cache.emplace(id, x, y, bitmap.width, bitmap.height, raster.glyph_width,
                                     raster.glyph_height, raster.descent, font);
                _atlas->updateRegion(x, y, bitmap.width, bitmap.height, bitmap.width,
                                     bitmap.data.toSpan().asBytes());

                for (usize i = 0; i < bitmap.height; i++) {
                    for (usize j = 0; j < bitmap.width; j++) {
                        if (bitmap.data[i * bitmap.width + j] != 0x3f) {
                            putchar('X');
                        } else {
                            putchar('.');
                        }
                    }
                    putchar('\n');
                }

                return _glyph_cache[_glyph_cache.count() - 1];
            }

            ui::WindowAppKit* _window;
            SpargelHostView* view_;
            resource::ResourceManager* _manager;
            ui::TextSystem* _text_system;
            base::unique_ptr<Device> _device;
            ShaderLibrary* _shader;
            RenderPipeline* _pipeline;
            Buffer* _positions;
            UniformData _uniform_data;
            Buffer* _uniforms;
            Buffer* _quads;
            Surface* _surface;
            CommandQueue* _queue;
            Texture* _atlas;
            base::vector<char> _str;
            base::vector<QuadData> _vquads;
            base::vector<GlyphEntry> _glyph_cache;
            // u32 _cur_row = 0;
            // u32 _cur_col = 0;
            // u32 _nxt_row = 0;
            bool _animating = false;
            Selection selection_{0};
            NSTextInputContext* input_context_;
            float text_width_ = 0;
            float anchor_offset_ = 0;
            base::vector<char> marked_;
            render::AtlasPacker packer_{TEXTURE_SIZE, TEXTURE_SIZE};
        };
        void demoMain() {
            auto platform = ui::makePlatform();
            auto resource_manager =
                resource::makeRelativeManager("../source/spargel/gpu/demo/resources");
            auto text_system = platform->createTextSystem();

            auto window = platform->makeWindow(500, 500);
            window->setTitle("Spargel Engine - GPU");

            Renderer r((ui::WindowAppKit*)window.get(), resource_manager.get(), text_system.get());

            platform->startLoop();
        }
    }  // namespace
}  // namespace spargel::gpu

int main() {
    spargel::gpu::demoMain();
    return 0;
}
