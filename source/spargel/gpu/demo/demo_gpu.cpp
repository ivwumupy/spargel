#include <spargel/base/logging.h>
#include <spargel/gpu/gpu.h>
#include <spargel/resource/directory.h>
#include <spargel/resource/resource.h>
#include <spargel/ui/ui.h>

using namespace spargel::gpu;

#define USE_VULKAN 1
#if SPARGEL_IS_MACOS
#define USE_METAL 1
#else
#define USE_METAL 0
#endif

#if USE_METAL
#include <spargel/gpu/demo/shader.metallib.inc>
#endif

#if USE_VULKAN
#include <spargel/gpu/demo/fragment_shader.spirv.inc>
#include <spargel/gpu/demo/vertex_shader.spirv.inc>
#endif

struct vertex_shader_info {
    static constexpr vertex_buffer_location position_buffer = {
        .apple = {0},
        .vulkan = {0},
    };
    static constexpr vertex_buffer_location quad_buffer = {
        .apple = {1},
        .vulkan = {1},
    };
    static constexpr vertex_buffer_location uniform_buffer = {
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

struct QuadData {
    struct {
        float x;
        float y;
    } origin;
    struct {
        float width;
        float height;
    } size;
    struct {
        float x;
        float y;
    } cell_origin;
};

u32 max(u32 a, u32 b) { return a > b ? a : b; }

class Renderer final : public spargel::ui::window_delegate {
public:
    explicit Renderer(spargel::ui::window* window,
                      spargel::resource::resource_manager* resource_manager,
                      spargel::ui::TextSystem* text_system)
        : _window{window}, _manager{resource_manager}, _text_system{text_system} {
        _window->set_delegate(this);

        _device = makeDevice(DeviceKind::metal);

        _surface = _device->createSurface(_window);

#if USE_METAL
        auto blob = _manager->open(spargel::resource::resource_id("shader.metallib"));

        _shader = _device->createShaderLibrary(blob->getSpan());

        blob->close();
#endif

        RenderPipelineBuilder builder;
        builder.setVertexShader(_shader, "vertex_shader");
        builder.setFragmentShader(_shader, "fragment_shader");
        builder.addVertexBuffer(sizeof(VertexPosition));
        builder.addVertexAttribute(VertexAttributeFormat::float32x2, 0, 0, 0);
        builder.addColorAttachment(TextureFormat::bgra8_unorm, false);
        _pipeline = builder.build(_device.get());

        _positions =
            _device->createBuffer(spargel::base::make_span<u8>(sizeof(positions), (u8*)positions));

        _uniform_data.viewport.width = _surface->width();
        _uniform_data.viewport.height = _surface->height();
        _uniforms = _device->createBuffer(
            spargel::base::make_span(sizeof(_uniform_data), (u8*)&_uniform_data));

        _queue = _device->createCommandQueue();

        _atlas = _device->createTexture(512, 512);

        _str.push('t');
    }

    ~Renderer() {
        _device->destroyBuffer(_positions);
        _device->destroyRenderPipeline(_pipeline);
        _device->destroyShaderLibrary(_shader);
    }

    void on_render() override {
        auto layout_result =
            _text_system->layoutLine(spargel::base::string_view(_str.begin(), _str.end()));

        _vquads.clear();
        float total_width = 0;
        for (usize r = 0; r < layout_result.runs.count(); r++) {
            auto& run = layout_result.runs[r];
            for (usize i = 0; i < run.glyphs.count(); i++) {
                auto& info = findOrInsert(run.glyphs[i]);
                QuadData quad;
                quad.origin.x = total_width + run.positions[i].x;
                quad.origin.y = info.descent + run.positions[i].y;
                quad.size.width = info.width - 1;
                quad.size.height = info.height;
                quad.cell_origin.x = info.x;
                quad.cell_origin.y = info.y;
                _vquads.push(quad);
            }
            total_width += run.width;
        }
        // todo: use uniform
        float half_width = total_width / 2;
        for (usize i = 0; i < _vquads.count(); i++) {
            _vquads[i].origin.x -= half_width;
        }
        _quads = _device->createBuffer(
            spargel::base::make_span(_vquads.count() * sizeof(QuadData), (u8*)_vquads.data()));

        auto texture = _surface->nextTexture();

        auto cmdbuf = _queue->createCommandBuffer();

        auto encoder = cmdbuf->beginRenderPass({
            .color_attachments = {{
                .texture = texture,
                .clear_color = {0.0, 1.0, 1.0, 1.0},
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
    }

    void on_keyboard(spargel::ui::keyboard_event& e) override {
        if (e.key == spargel::ui::physical_key::key_delete &&
            e.action == spargel::ui::keyboard_action::press) {
            _str.pop();
        } else if (e.key == spargel::ui::physical_key::space &&
                   e.action == spargel::ui::keyboard_action::press) {
            _animating = !_animating;
            _window->setAnimating(_animating);
        } else {
            _str.push(e.toChar());
            if (!_animating) {
                _window->requestRedraw();
            }
        }
    }

private:
    struct GlyphEntry {
        spargel::ui::GlyphId id;
        u32 x;
        u32 y;
        u32 width;
        u32 height;
        float glyph_width;
        float glyph_height;
        float descent;
    };

    // todo: optimize
    GlyphEntry& findOrInsert(spargel::ui::GlyphId id) {
        for (usize i = 0; i < _glyph_cache.count(); i++) {
            if (_glyph_cache[i].id == id) {
                return _glyph_cache[i];
            }
        }
        // not found
        auto raster = _text_system->rasterizeGlyph(id);
        auto& bitmap = raster.bitmap;
        // try to insert
        if (_cur_col + bitmap.width >= 512) {
            // next row
            _cur_row = _nxt_row;
            _cur_col = 0;
        }
        u32 x = _cur_col;
        _cur_col += bitmap.width;
        _nxt_row = max(_nxt_row, _cur_row + bitmap.height);
        if (_nxt_row >= 512) {
            spargel_log_fatal("glyph atlas out of space!");
            spargel_panic_here();
        }
        u32 y = _cur_row;
        _glyph_cache.push(id, x, y, bitmap.width, bitmap.height, raster.glyph_width,
                          raster.glyph_height, raster.descent);
        _atlas->updateRegion(x, y, bitmap.width, bitmap.height, bitmap.width, bitmap.data.toSpan());
        return _glyph_cache[_glyph_cache.count() - 1];
    }

    spargel::ui::window* _window;
    spargel::resource::resource_manager* _manager;
    spargel::ui::TextSystem* _text_system;
    spargel::base::unique_ptr<Device> _device;
    ObjectPtr<ShaderLibrary> _shader;
    ObjectPtr<RenderPipeline> _pipeline;
    ObjectPtr<Buffer> _positions;
    UniformData _uniform_data;
    ObjectPtr<Buffer> _uniforms;
    ObjectPtr<Buffer> _quads;
    ObjectPtr<Surface> _surface;
    ObjectPtr<CommandQueue> _queue;
    ObjectPtr<Texture> _atlas;
    spargel::base::vector<char> _str;
    spargel::base::vector<QuadData> _vquads;
    spargel::base::vector<GlyphEntry> _glyph_cache;
    u32 _cur_row = 0;
    u32 _cur_col = 0;
    u32 _nxt_row = 0;
    bool _animating = true;
};

int main() {
    auto platform = spargel::ui::make_platform();
    auto resource_manager = spargel::resource::make_relative_manager();
    auto text_system = platform->createTextSystem();

    auto window = platform->make_window(500, 500);
    window->set_title("Spargel Engine - GPU");

    Renderer r(window.get(), resource_manager.get(), text_system.get());

    platform->start_loop();
    return 0;
}
