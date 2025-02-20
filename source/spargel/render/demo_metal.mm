#include <spargel/base/algorithm.h>
#include <spargel/base/assert.h>
#include <spargel/base/logging.h>
#include <spargel/base/optional.h>
#include <spargel/base/tag_invoke.h>
#include <spargel/math/vector.h>
#include <spargel/resource/directory.h>
#include <spargel/ui/platform.h>
#include <spargel/ui/window.h>
#include <spargel/ui/ui_mac.h>

//
#import <Metal/Metal.h>
#import <simd/simd.h>

struct Vector3f final {
    float x;
    float y;
    float z;

    Vector3f cross(Vector3f const& other) const {
        return {
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x,
        };
    }

    Vector3f& operator+=(Vector3f const& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    void normalize() {
        float l = length();
        x /= l;
        y /= l;
        z /= l;
    }

    float length() const {
        return sqrtf(x * x + y * y + z * z);
    }
};

struct Point3f final {
    float x;
    float y;
    float z;

    Vector3f to(Point3f const& other) const {
        return {
            other.x - x, other.y - y, other.z - z,
        };
    }
};

class Cursor final {
public:
    explicit Cursor(spargel::base::span<u8> bytes)
        : _begin{bytes.begin()}, _end{bytes.end()}, _cur{_begin} {
        printf("len = %zu\n", bytes.count());
    }

    spargel::base::Optional<u32> readU32() {
        eatWhitespace();
        u32 result = 0;
        u8 c = peek();
        if (c < '0' || c > '9') {
            return {};
        }
        while (!isEnd()) {
            c = peek();
            if (c < '0' || c > '9') {
                break;
            }
            result = result * 10 + (c - '0');
            advance();
        }
        return spargel::base::makeOptional<u32>(result);
    }

    spargel::base::Optional<float> readFloat() {
        eatWhitespace();
        float result = 0;
        float d = 1;
        u8 c = peek();
        if ((c < '0' || c > '9') && (c != '-')) {
            return {};
        }
        bool negative = false;
        if (c == '-') {
            negative = true;
            advance();
        }
        bool is_decimal = false;
        while (!isEnd()) {
            c = peek();
            if (c == '.') {
                is_decimal = true;
                advance();
                continue;
            }
            if (c < '0' || c > '9') {
                break;
            }
            u8 digit = c - '0';
            if (is_decimal) {
                d *= 10;
                result += digit / d;
            } else {
                result = result * 10 + digit;
            }
            advance();
        }
        if (negative) {
            result = -result;
        }
        return spargel::base::makeOptional<float>(result);
    }

private:
    bool isEnd() const { return _cur >= _end; }

    u8 peek() const {
        if (isEnd()) {
            return 0;
        }
        return *_cur;
    }

    void advance() {
        if (isEnd()) {
            return;
        }
        _cur++;
    }

    void eatWhitespace() {
        while (!isEnd()) {
            u8 c = peek();
            if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
                advance();
                continue;
            } else if (c == '#') {
                eatUntil('\n');
            } else {
                break;
            }
        }
    }

    void eatUntil(u8 ch) {
        while (!isEnd()) {
            u8 c = peek();
            if (c == ch) {
                break;
            }
            advance();
        }
    }

    u8 const* _begin;
    u8 const* _end;
    u8 const* _cur;
};

float min(float x, float y) { return x < y ? x : y; }
float max(float x, float y) { return x > y ? x : y; }

struct BoundingBox {
    float xmin;
    float ymin;
    float zmin;
    float xmax;
    float ymax;
    float zmax;

    BoundingBox() = default;
    BoundingBox(float x, float y, float z) : xmin{x}, ymin{y}, zmin{z}, xmax{x}, ymax{y}, zmax{z} {}

    void merge(float x, float y, float z) {
        xmin = min(xmin, x);
        ymin = min(ymin, y);
        zmin = min(zmin, z);
        xmax = max(xmax, x);
        ymax = max(ymax, y);
        zmax = max(zmax, z);
    }
};

class SimpleMesh final {
public:
    static spargel::base::Optional<SimpleMesh> create(spargel::base::span<u8> bytes) {
        Cursor cursor(bytes);

        auto face_count = cursor.readU32();
        auto vert_count = cursor.readU32();

        if (!face_count.hasValue() || !vert_count.hasValue()) {
            return {};
        }

        spargel::base::vector<u32> indices;
        spargel::base::vector<Point3f> vertices;

        u32 indices_count = face_count.value() * 3;
        indices.reserve(indices_count);
        indices.set_count(indices_count);

        for (u32 i = 0; i < indices_count; i++) {
            auto value = cursor.readU32();
            if (!value.hasValue()) {
                spargel_log_error("bad index %d", i);
                return {};
            }
            indices[i] = value.value();
        }

        vertices.reserve(vert_count.value());
        vertices.set_count(vert_count.value());

        for (u32 i = 0; i < vert_count.value(); i++) {
            auto x = cursor.readFloat();
            auto y = cursor.readFloat();
            auto z = cursor.readFloat();
            if (!x.hasValue() || !y.hasValue() || !z.hasValue()) {
                spargel_log_error("bad point %d", i);
                return {};
            }
            // vertices[i] = {x.value(), y.value(), z.value()};
            vertices[i] = {x.value() * 1000, y.value() * 1000, z.value() * 1000};
        }

        // spargel_assert(vertices.getAllocator() != spargel::base::default_allocator());
        // spargel_assert(indices.getAllocator() == spargel::base::default_allocator());

        return spargel::base::makeOptional<SimpleMesh>(face_count.value(), vert_count.value(),
                                                       spargel::base::move(indices),
                                                       spargel::base::move(vertices));
    }

    SimpleMesh() = default;

    SimpleMesh(u32 face_count, u32 vert_count, spargel::base::vector<u32> indices,
               spargel::base::vector<Point3f> vertices)
        : _face_count{face_count},
          _vert_count{vert_count},
          _indices(spargel::base::move(indices)),
          _vertices(spargel::base::move(vertices)) {
        _bbox = BoundingBox(_vertices[0].x, _vertices[0].y, _vertices[0].z);
        for (usize i = 0; i < _vertices.count(); i++) {
            _bbox.merge(_vertices[i].x, _vertices[i].y, _vertices[i].z);
        }
        computeNormals();
    }

    SimpleMesh(SimpleMesh const& other) = default;
    SimpleMesh& operator=(SimpleMesh const& other) {
        SimpleMesh tmp(other);
        spargel::base::swap(*this, tmp);
        return *this;
    }

    SimpleMesh(SimpleMesh&& other) { spargel::base::swap(*this, other); }
    SimpleMesh& operator=(SimpleMesh&& other) {
        SimpleMesh tmp(spargel::base::move(other));
        spargel::base::swap(*this, tmp);
        return *this;
    }

    u32 getFaceCount() const { return _face_count; }
    u32 getVertexCount() const { return _vert_count; }

    spargel::base::span<u32> getIndices() const { return _indices.toSpan(); }
    spargel::base::span<Point3f> getVertices() const { return _vertices.toSpan(); }
    spargel::base::span<Vector3f> getNormals() const { return _normals.toSpan(); }

    BoundingBox const& getBoundingBox() const { return _bbox; }

    friend void tag_invoke(spargel::base::tag<spargel::base::swap>, SimpleMesh& lhs,
                           SimpleMesh& rhs) {
        spargel::base::swap(lhs._face_count, rhs._face_count);
        spargel::base::swap(lhs._vert_count, rhs._vert_count);
        spargel::base::swap(lhs._indices, rhs._indices);
        spargel::base::swap(lhs._vertices, rhs._vertices);
        spargel::base::swap(lhs._normals, rhs._normals);
        spargel::base::swap(lhs._bbox, rhs._bbox);
    }

private:
    void computeNormals() {
        _normals.reserve(_vertices.count());
        _normals.set_count(_vertices.count());

        for (usize i = 0; i < _indices.count(); i += 3) {
            Point3f const& x = _vertices[_indices[i]];
            Point3f const& y = _vertices[_indices[i + 1]];
            Point3f const& z = _vertices[_indices[i + 2]];

            _normals[_indices[i]] += x.to(y).cross(x.to(z));
            _normals[_indices[i+1]] += y.to(z).cross(y.to(x));
            _normals[_indices[i+2]] += z.to(x).cross(z.to(y));
        }

        // normalize
        for (usize i = 0; i < _normals.count(); i++) {
            auto l = _normals[i].length();
            if (l < 0.001) {
                spargel_log_info("small normal: %zu", i);
            }
            _normals[i].normalize();
        }
    }

    u32 _face_count = 0;
    u32 _vert_count = 0;
    spargel::base::vector<u32> _indices;
    spargel::base::vector<Point3f> _vertices;
    spargel::base::vector<Vector3f> _normals;
    BoundingBox _bbox;
};

struct ControlData {
    float model_to_world[16];
    float world_to_camera[16];
    float camera_to_clip[16];
};

class Delegate final : public spargel::ui::WindowDelegate {
public:
    Delegate(spargel::ui::WindowAppKit* window) : _window{window} { _Setup(); }

    void onKeyboard(spargel::ui::KeyboardEvent& e) override {
        float d = 10;
        if (e.key == spargel::ui::PhysicalKey::key_w) {
            _cd.world_to_camera[13] += d;
        } else if (e.key == spargel::ui::PhysicalKey::key_s) {
            _cd.world_to_camera[13] -= d;
        } else if (e.key == spargel::ui::PhysicalKey::key_a) {
            _cd.world_to_camera[12] -= d;
        } else if (e.key == spargel::ui::PhysicalKey::key_d) {
            _cd.world_to_camera[12] += d;
        } else if (e.key == spargel::ui::PhysicalKey::key_j) {
            _cd.world_to_camera[14] += d;
        } else if (e.key == spargel::ui::PhysicalKey::key_k) {
            _cd.world_to_camera[14] -= d;
        }
        // spargel_log_info("camera: (%.1f, %.1f, 0); scale: %.2f", _cd.camera[0], _cd.camera[1],
        //                  _cd.scale);
    }

    void onRender() override {
        auto cmdbuf = [_queue commandBuffer];

        auto drawable = [_layer nextDrawable];

        auto desc = [MTLRenderPassDescriptor renderPassDescriptor];
        desc.colorAttachments[0].texture = drawable.texture;
        desc.colorAttachments[0].loadAction = MTLLoadActionClear;
        desc.colorAttachments[0].clearColor = MTLClearColorMake(0.0, 1.0, 1.0, 1.0);

        auto encoder = [cmdbuf renderCommandEncoderWithDescriptor:desc];

        [encoder setRenderPipelineState:_mesh_pipeline];

        [encoder setCullMode:MTLCullModeBack];

        [encoder setVertexBuffer:_vertices offset:0 atIndex:0];
        [encoder setVertexBuffer:_normals offset:0 atIndex:1];

        float ratio = _layer.drawableSize.width / _layer.drawableSize.height;
        constexpr float alpha = 3.14159 / 6;
        float height = tanf(alpha);
        float width = height * ratio;
        _cd.camera_to_clip[0] = 2.0 / width;
        _cd.camera_to_clip[5] = 2.0 / height;
        [encoder setVertexBytes:&_cd length:sizeof(ControlData) atIndex:2];

        [encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                            indexCount:_mesh.getIndices().count()
                             indexType:MTLIndexTypeUInt32
                           indexBuffer:_indices
                     indexBufferOffset:0];

        [encoder endEncoding];

        [cmdbuf presentDrawable:drawable];
        [cmdbuf commit];
    }

private:
    void _Setup() {
        NSError* error;

        _resource = spargel::resource::make_relative_manager();

        {
            auto blob = _resource->open(spargel::resource::resource_id("bunny.smesh"));
            auto s = SimpleMesh::create(blob->getSpan());
            if (!s.hasValue()) {
                spargel_log_error("wrong format");
                spargel_panic_here();
            }
            _mesh = spargel::base::move(s).value();

            auto& bbox = _mesh.getBoundingBox();
            spargel_log_info("(%.3f, %.3f, %.3f) - (%.3f, %.3f, %.3f)", bbox.xmin, bbox.ymin,
                             bbox.zmin, bbox.xmax, bbox.ymax, bbox.zmax);
        }

        _device = MTLCreateSystemDefaultDevice();
        _queue = [_device newCommandQueue];

        {
            _layer = (CAMetalLayer*)_window->getHandle().apple.layer;
            _layer.device = _device;
        }

        {
            auto blob = _resource->open(spargel::resource::resource_id("shader.metallib"));
            auto blob_span = blob->getSpan();
            auto library_data =
                dispatch_data_create(blob_span.data(), blob_span.count(), dispatch_get_main_queue(),
                                     DISPATCH_DATA_DESTRUCTOR_DEFAULT);

            _library = [_device newLibraryWithData:library_data error:&error];

            _vert_func = [_library newFunctionWithName:@"vertex_shader"];
            _frag_func = [_library newFunctionWithName:@"fragment_shader"];
        }

        {
            auto vert_desc = [MTLVertexDescriptor vertexDescriptor];
            // Buffer 0 is vertices.
            vert_desc.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
            vert_desc.layouts[0].stride = sizeof(Point3f);
            // Buffer 1 is normals.
            vert_desc.layouts[1].stepFunction = MTLVertexStepFunctionPerVertex;
            vert_desc.layouts[1].stride = sizeof(Vector3f);
            // Attribute 0 is vertex position.
            vert_desc.attributes[0].format = MTLVertexFormatFloat3;
            vert_desc.attributes[0].offset = 0;
            vert_desc.attributes[0].bufferIndex = 0;
            vert_desc.attributes[1].format = MTLVertexFormatFloat3;
            vert_desc.attributes[1].offset = 0;
            vert_desc.attributes[1].bufferIndex = 1;

            auto desc = [[MTLRenderPipelineDescriptor alloc] init];
            desc.vertexFunction = _vert_func;
            desc.fragmentFunction = _frag_func;
            desc.vertexDescriptor = vert_desc;

            desc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
            // desc.colorAttachments[0].blendingEnabled = false;

            desc.inputPrimitiveTopology = MTLPrimitiveTopologyClassTriangle;

            _mesh_pipeline = [_device newRenderPipelineStateWithDescriptor:desc error:&error];
        }

        {
            _vertices = [_device newBufferWithBytes:_mesh.getVertices().asBytes().data()
                                             length:_mesh.getVertices().asBytes().count()
                                            options:MTLResourceStorageModeShared];
            _indices = [_device newBufferWithBytes:_mesh.getIndices().asBytes().data()
                                            length:_mesh.getIndices().asBytes().count()
                                           options:MTLResourceStorageModeShared];
            _normals = [_device newBufferWithBytes:_mesh.getNormals().asBytes().data()
                                            length:_mesh.getNormals().asBytes().count()
                                           options:MTLResourceStorageModeShared];
        }
    }

    spargel::ui::WindowAppKit* _window;
    spargel::base::unique_ptr<spargel::resource::directory_resource_manager> _resource;

    SimpleMesh _mesh;

    id<MTLDevice> _device;
    id<MTLCommandQueue> _queue;
    id<MTLRenderPipelineState> _mesh_pipeline;
    id<MTLLibrary> _library;
    id<MTLFunction> _vert_func;
    id<MTLFunction> _frag_func;

    id<MTLBuffer> _vertices;
    id<MTLBuffer> _indices;
    id<MTLBuffer> _normals;

    CAMetalLayer* _layer;

    static constexpr float zmax = 1000;
    static constexpr float zmin = 1;

    // clang-format off
    ControlData _cd = {
        .model_to_world = {1, 0, 0, 0,
                           0, 1, 0, 0,
                           0, 0, 1, 0,
                           0, 0, 500, 1,},
        .world_to_camera = {1, 0, 0, 0,
                            0, 1, 0, 0,
                            0, 0, 1, 0,
                            0, 0, 0, 1,},
        .camera_to_clip = {1, 0, 0, 0,
                           0, 1, 0, 0,
                           0, 0, zmax/(zmax-zmin), 1,
                           0, 0, -zmax*zmin/(zmax-zmin), 0,},
    };
    // clang-format on
};

int main() {
    auto platform = spargel::ui::makePlatform();

    auto window = platform->makeWindow(500, 500);
    window->setTitle("Spargel Demo - Render");

    Delegate d(static_cast<spargel::ui::WindowAppKit*>(window.get()));
    window->setDelegate(&d);

    platform->startLoop();
    return 0;
}
