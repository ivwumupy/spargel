
#include <spargel/base/logging.h>
#include <spargel/config.h>
#include <spargel/resource/directory.h>
#include <spargel/ui/ui.h>

// libc
#include <stdlib.h>

// GLAD
#include <glad/gl.h>

#if SPARGEL_IS_LINUX
#include <GL/glx.h>
#elif SPARGEL_IS_WINDOWS
#include <windows.h>
#else
#error unimplemented
#endif

namespace spargel {

    class delegate final : public ui::window_delegate {
    public:
        void on_render() override {
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(program);

            glBindVertexArray(vao);

            glDrawArrays(GL_TRIANGLES, 0, 3);

#if SPARGEL_IS_LINUX
            glXSwapBuffers(_x_display, _x_window);
#elif SPARGEL_IS_WINDOWS
            wglSwapLayerBuffers(_win32_hdc, WGL_SWAP_MAIN_PLANE);
#endif
        }

        unsigned int program;
        unsigned int vao;

#if SPARGEL_IS_LINUX
        Display* _x_display;
        int _x_window;
#elif SPARGEL_IS_WINDOWS
        HDC _win32_hdc;
#endif
    };

    extern "C" int main() {
        auto resource_manager = resource::make_relative_manager();

        auto platform = ui::make_platform();
        auto window = platform->make_window(800, 600);
        delegate d;
        window->set_delegate(&d);
        window->set_title("Spargel Demo - OpenGL");

        auto handle = window->handle();

#if SPARGEL_IS_LINUX
        auto x_window = handle.xcb.window;
        auto x_display = (Display*)handle.xcb.display;
        auto x_visual_info = (XVisualInfo*)handle.xcb.visual_info;

        d._x_display = x_display;
        d._x_window = x_window;

        GLXContext glx_ctx = glXCreateContext(x_display, x_visual_info, nullptr, GL_TRUE);
        if (!glx_ctx) {
            spargel_log_fatal("cannot create GL context");
            spargel_panic_here();
        }

        if (!glXMakeCurrent(x_display, x_window, glx_ctx)) {
            spargel_log_fatal("cannot make GL context current");
            spargel_panic_here();
        }
#elif SPARGEL_IS_WINDOWS
        auto hwnd = (HWND)handle.win32.hwnd;
        PIXELFORMATDESCRIPTOR pfd = {
            sizeof(PIXELFORMATDESCRIPTOR),
            1,
            PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,  // Flags
            PFD_TYPE_RGBA,  // The kind of framebuffer. RGBA or palette.
            32,             // Colordepth of the framebuffer.
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            24,  // Number of bits for the depthbuffer
            8,   // Number of bits for the stencilbuffer
            0,   // Number of Aux buffers in the framebuffer.
            PFD_MAIN_PLANE,
            0,
            0,
            0,
            0};

        HDC win32_hdc = GetDC(hwnd);
        d._win32_hdc = win32_hdc;

        int pixelFormat;
        pixelFormat = ChoosePixelFormat(win32_hdc, &pfd);
        SetPixelFormat(win32_hdc, pixelFormat, &pfd);

        HGLRC wgl_ctx = wglCreateContext(win32_hdc);

        wglMakeCurrent(win32_hdc, wgl_ctx);
#endif

        spargel_log_info("Glad generator version: %s", GLAD_GENERATOR_VERSION);

        gladLoaderLoadGL();

        int success;
        char infoLog[512];

        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        auto vertexShaderCode = resource_manager->open(resource::resource_id("demo_opengl.vs"));
        if (!vertexShaderCode) {
            spargel_log_fatal("cannot open vertex shader file");
            spargel_panic_here();
        }
        auto pVertexShaderCode = (GLchar*)vertexShaderCode->map_data();
        auto vertexShaderCodeLength = (GLint)vertexShaderCode->size();
        glShaderSource(vertexShader, 1, &pVertexShaderCode, &vertexShaderCodeLength);
        vertexShaderCode->close();
        delete vertexShaderCode;
        glCompileShader(vertexShader);
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
            spargel_log_fatal("vertex shader compile error: %s", infoLog);
            spargel_panic_here();
        }

        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        auto fragmentShaderCode = resource_manager->open(resource::resource_id("demo_opengl.fs"));
        if (!fragmentShaderCode) {
            spargel_log_fatal("cannot open fragment shader file");
            spargel_panic_here();
        }
        auto pFragmentShaderCode = (GLchar*)fragmentShaderCode->map_data();
        auto fragmentShaderCodeLength = (GLint)fragmentShaderCode->size();
        glShaderSource(fragmentShader, 1, &pFragmentShaderCode, &fragmentShaderCodeLength);
        fragmentShaderCode->close();
        delete fragmentShaderCode;
        glCompileShader(fragmentShader);
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
            spargel_log_fatal("fragment shader compile error: %s", infoLog);
            spargel_panic_here();
        }

        unsigned int shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
            spargel_log_fatal("shader program link error: %s", infoLog);
            spargel_panic_here();
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        d.program = shaderProgram;

        float vertices[] = {
            // positions         // colors
            0.5f,  -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom right
            -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,  // bottom left
            0.0f,  0.5f,  0.0f, 0.0f, 0.0f, 1.0f   // top
        };

        unsigned int vao, vbo;
        glGenVertexArrays(1, &vao);

        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                              (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindVertexArray(0);

        d.vao = vao;

        platform->start_loop();

        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteProgram(shaderProgram);

        gladLoaderUnloadGL();

#if SPARGEL_IS_LINUX
        glXDestroyContext(x_display, glx_ctx);
#elif SPARGEL_IS_WINDOWS
        wglDeleteContext(wgl_ctx);
#endif

        resource_manager->close();

        return 0;
    }

}  // namespace spargel
