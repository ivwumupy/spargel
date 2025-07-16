#include <spargel/base/logging.h>
#include <spargel/base/unique_ptr.h>
#include <spargel/base/vector.h>
#include <spargel/codec/model/gltf.h>
#include <spargel/config.h>
#include <spargel/resource/directory.h>
#include <spargel/ui/platform.h>
#include <spargel/ui/window.h>

using namespace spargel;
using namespace spargel::codec::model;
using namespace spargel::base::literals;

// GLAD
#include <glad/gl.h>

#if SPARGEL_IS_LINUX
#include <GL/glx.h>
#else
#error unimplemented
#endif

namespace {

    struct Instance {
        u32 vao;
        bool indices;
        int indices_type;
        int mode;
        usize count;
    };

    struct Context {
        u32 program;
        base::vector<u32> buffers;
        base::vector<Instance> instances;
    };

    class Delegate final : public ui::WindowDelegate {
    public:
        void onRender() override {
            glClearColor(0.1f, 0.0f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(program);

            for (auto instance : instances) {
                glBindVertexArray(instance.vao);

                if (instance.indices) {
                    glDrawElements(instance.mode, instance.count, instance.indices_type, (void*)0);
                } else {
                    glDrawArrays(instance.mode, 0, instance.count);
                };
            }

#if SPARGEL_IS_LINUX
            glXSwapBuffers(x_display, x_window);
#endif
        }

        u32 program;
        base::vector<Instance> instances;

#if SPARGEL_IS_LINUX
        Display* x_display;
        int x_window;
#endif
    };

}  // namespace

void loadProgram(resource::ResourceManager* resource_manager, Context& ctx) {
    int success;
    char infoLog[512];

    u32 vertexShader = glCreateShader(GL_VERTEX_SHADER);
    auto vertexShaderCode = resource_manager->open(resource::ResourceId("demo_gltf_opengl.vs"_sv));
    if (!vertexShaderCode.hasValue()) {
        spargel_log_fatal("cannot open vertex shader file");
        spargel_panic_here();
    }
    auto pVertexShaderCode = (GLchar*)vertexShaderCode.value()->mapData();
    auto vertexShaderCodeLength = (GLint)vertexShaderCode.value()->size();
    glShaderSource(vertexShader, 1, &pVertexShaderCode, &vertexShaderCodeLength);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        spargel_log_fatal("vertex shader compile error: %s", infoLog);
        spargel_panic_here();
    }

    u32 fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    auto fragmentShaderCode = resource_manager->open(resource::ResourceId("demo_gltf_opengl.fs"_sv));
    if (!fragmentShaderCode.hasValue()) {
        spargel_log_fatal("cannot open fragment shader file");
        spargel_panic_here();
    }
    auto pFragmentShaderCode = (GLchar*)fragmentShaderCode.value()->mapData();
    auto fragmentShaderCodeLength = (GLint)fragmentShaderCode.value()->size();
    glShaderSource(fragmentShader, 1, &pFragmentShaderCode, &fragmentShaderCodeLength);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        spargel_log_fatal("fragment shader compile error: %s", infoLog);
        spargel_panic_here();
    }

    u32 program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        spargel_log_fatal("shader program link error: %s", infoLog);
        spargel_panic_here();
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    ctx.program = program;
}

void loadModel(resource::ResourceManager* resource_manager, const GlTF& gltf, Context& ctx) {
    // show meta info
    auto& asset = gltf.asset;
    spargel_log_info("glTF version: \"%s\"", base::CString(asset.version).data());
    if (asset.copyright.hasValue())
        spargel_log_info("glTF copyright: \"%s\"", base::CString(asset.copyright.value()).data());
    if (asset.generator.hasValue())
        spargel_log_info("glTF generator: \"%s\"", base::CString(asset.generator.value()).data());
    if (asset.minVersion.hasValue())
        spargel_log_info("glTF minVersion: \"%s\"", base::CString(asset.minVersion.value()).data());

    // load raw buffers
    base::vector<base::unique_ptr<resource::Resource>> buffer_resources;
    if (gltf.buffers.hasValue()) {
        for (auto& buffer : gltf.buffers.value()) {
            spargel_check(buffer.uri.hasValue());

            auto optional = resource_manager->open(resource::ResourceId(buffer.uri.value()));
            spargel_check(optional.hasValue());
            buffer_resources.emplace(base::move(optional.value()));
        }
    }
    spargel_log_info("glTF buffer count: %zu", buffer_resources.count());

    base::vector<base::span<u8>> raw_buffers;
    for (auto& resource : buffer_resources) {
        raw_buffers.emplace(resource->getSpan());
    }

    // upload buffers
    if (gltf.bufferViews.hasValue()) {
        spargel_log_info("glTF bufferView count: %zu", gltf.bufferViews.value().count());
        for (auto& bufferView : gltf.bufferViews.value()) {
            int byteOffset = bufferView.byteOffset.hasValue() ? bufferView.byteOffset.value() : 0;
            int target = bufferView.target.hasValue() ? bufferView.target.value() : GL_ARRAY_BUFFER;

            u32 vbo;
            glGenBuffers(1, &vbo);
            glBindBuffer(target, vbo);
            glBufferData(target, bufferView.byteLength, raw_buffers[bufferView.buffer].data() + byteOffset, GL_STATIC_DRAW);

            ctx.buffers.emplace(vbo);
        }
    }

    base::vector<GlTFAccessor> accessors;
    if (gltf.accessors.hasValue()) {
        accessors = gltf.accessors.value();
    }
    spargel_log_info("glTF accessor count: %zu", accessors.count());

    // bind vertex pointers
    base::vector<base::vector<Instance>> instances;
    if (gltf.meshes.hasValue()) {
        spargel_log_info("glTF mesh count: %zu", gltf.meshes.value().count());
        for (auto& mesh : gltf.meshes.value()) {
            /*
            if (mesh.name.hasValue()) {
                spargel_log_debug("mesh name: %s", base::CString(mesh.name.value()).data());
            }
            */

            base::vector<Instance> primitive_instances;
            for (auto& primitive : mesh.primitives) {
                u32 vao;
                glGenVertexArrays(1, &vao);
                glBindVertexArray(vao);

                auto& attributes = primitive.attributes;

                spargel_check(attributes.position.hasValue());
                auto& position_accessor = accessors[attributes.position.value()];
                spargel_check(position_accessor.bufferView.hasValue());
                spargel_check(position_accessor.componentType == GL_FLOAT);
                spargel_check(position_accessor.type == base::String("VEC3"));
                glBindBuffer(GL_ARRAY_BUFFER, ctx.buffers[position_accessor.bufferView.value()]);
                glVertexAttribPointer(
                    0,
                    3,
                    GL_FLOAT,
                    GL_FALSE,
                    3 * sizeof(float),
                    reinterpret_cast<void*>(position_accessor.byteOffset.hasValue() ? position_accessor.byteOffset.value() : 0));
                glEnableVertexAttribArray(0);

                spargel_check(attributes.normal.hasValue());
                auto& normal_accessor = accessors[attributes.normal.value()];
                spargel_check(normal_accessor.bufferView.hasValue());
                spargel_check(normal_accessor.componentType == GL_FLOAT);
                spargel_check(normal_accessor.type == base::String("VEC3"));
                glBindBuffer(GL_ARRAY_BUFFER, ctx.buffers[normal_accessor.bufferView.value()]);
                glVertexAttribPointer(
                    1,
                    3,
                    GL_FLOAT,
                    GL_FALSE,
                    3 * sizeof(float),
                    reinterpret_cast<void*>(normal_accessor.byteOffset.hasValue() ? normal_accessor.byteOffset.value() : 0));
                glEnableVertexAttribArray(1);

                Instance instance;
                instance.vao = vao;
                instance.count = position_accessor.count;
                instance.mode = primitive.mode.hasValue() ? primitive.mode.value() : GL_TRIANGLES;
                instance.indices = false;

                if (primitive.indices.hasValue()) {
                    auto& indices_accesor = accessors[primitive.indices.value()];
                    spargel_check(indices_accesor.bufferView.hasValue());
                    spargel_check(indices_accesor.type == base::String("SCALAR"));
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx.buffers[indices_accesor.bufferView.value()]);

                    instance.indices_type = indices_accesor.componentType;
                    instance.indices = true;
                }

                primitive_instances.emplace(instance);
                ctx.instances.emplace(instance);

                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindVertexArray(0);
            }

            instances.emplace(base::move(primitive_instances));
        }
    }

    int scene_index = 0;
    if (gltf.scene.hasValue()) {
        scene_index = gltf.scene.value();
    }
    spargel_log_info("glTF scene: %d", scene_index);

    if (gltf.scenes.hasValue()) {
        spargel_log_info("glTF scene count: %zu", gltf.scenes.value().count());
        [[maybe_unused]] auto& scene = gltf.scenes.value()[scene_index];
    }

    /*
    float position_data[] = {
        0.5f, -0.5f, 0.0f,   // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        0.0f, 0.5f, 0.0f,    // top
    };
    float normal_data[] = {
        1.0f, 0.0f, 0.0f,  // bottom right
        0.0f, 1.0f, 0.0f,  // bottom left
        0.0f, 0.0f, 1.0f,  // top
    };
    u32 vao, vbo_p, vbo_n;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo_p);
    glGenBuffers(1, &vbo_n);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_p);
    glBufferData(GL_ARRAY_BUFFER, sizeof(position_data), position_data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_n);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normal_data), normal_data, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    ctx.instances.emplace(vao, GL_TRIANGLES, 3);
    */
}

void cleanup(Context& ctx) {
    for (auto& instance : ctx.instances) {
        glDeleteVertexArrays(1, &instance.vao);
    }

    glDeleteBuffers(ctx.buffers.count(), ctx.buffers.data());

    glDeleteProgram(ctx.program);
}

int main() {
    auto resource_manager = resource::makeRelativeManager("resources"_sv);

    // load glTF
    GlTF gltf;
    {
        auto filename = "scene.gltf"_sv;
        auto optional = resource_manager->open(resource::ResourceId(filename));
        if (!optional.hasValue()) {
            spargel_log_fatal("Cannot open file \"%s\"\n", base::CString(filename).data());
            return 1;
        }
        auto& resource = optional.value();

        auto result = parseGlTF((char*)resource->mapData(), resource->size());
        if (result.isLeft()) {
            gltf = base::move(result.left());
        } else {
            spargel_log_fatal("Failed to parse glTF: %s\n", base::CString(result.right().message()).data());
            return 1;
        }
    }

    auto platform = ui::makePlatform();
    auto window = platform->makeWindow(800, 600);
    Delegate delegate;
    window->setDelegate(&delegate);
    window->setTitle("Spargel Demo - glTF Viewer (OpenGL)");

    auto handle = window->getHandle();

#if SPARGEL_IS_LINUX
    auto x_window = handle.xcb.window;
    auto x_display = (Display*)handle.xcb.display;
    auto x_visual_info = (XVisualInfo*)handle.xcb.visual_info;

    delegate.x_display = x_display;
    delegate.x_window = x_window;

    GLXContext glx_ctx = glXCreateContext(x_display, x_visual_info, nullptr, GL_TRUE);
    if (!glx_ctx) {
        spargel_log_fatal("cannot create GL context");
        spargel_panic_here();
    }

    if (!glXMakeCurrent(x_display, x_window, glx_ctx)) {
        spargel_log_fatal("cannot make GL context current");
        spargel_panic_here();
    }
#endif

    spargel_log_info("Glad generator version: %s", GLAD_GENERATOR_VERSION);

    gladLoaderLoadGL();

    // load OpenGL-related objects
    Context ctx;
    loadProgram(resource_manager.get(), ctx);
    delegate.program = ctx.program;

    // load model from glTF
    loadModel(resource_manager.get(), gltf, ctx);
    delegate.instances = ctx.instances;

    glDisable(GL_DEPTH_TEST);

    platform->startLoop();

    cleanup(ctx);

    gladLoaderUnloadGL();

#if SPARGEL_IS_LINUX
    glXDestroyContext(x_display, glx_ctx);
#endif

    return 0;
}
