#include <spargel/base/string_view.h>
#include <spargel/base/unique_ptr.h>
#include <spargel/gpu/gpu_context.h>
#include <spargel/render/ui_renderer.h>
#include <spargel/render/ui_scene.h>
#include <spargel/resource/directory.h>
#include <spargel/text/font.h>
#include <spargel/text/styled_text.h>
#include <spargel/text/text_shaper.h>
#include <spargel/ui/platform.h>
#include <spargel/ui/view_host.h>
#include <spargel/ui/window.h>

//
#include <stdio.h>

namespace spargel::render {
    namespace {
        using namespace base::literals;
        gpu::GPUBackend inferBackend() {
#if SPARGEL_IS_MACOS
            return gpu::GPUBackend::metal;
#endif
            spargel_panic("cannot determine backend");
        }
        class Demo final : public ui::WindowDelegate {
        public:
            Demo() {
                initRenderer();
                initWindow();
                font_ = text::createDefaultFont();
                prepareScene();
            }
            void start() { platform_->startLoop(); }
            void onRender() override { renderer_->render(scene_); }

        private:
            void initRenderer() {
                gpu_context_ = gpu::makeContext(inferBackend());
                shaper_ = text::TextShaper::create();
                renderer_ = render::makeUIRenderer(gpu_context_.get(), shaper_.get());
            }
            void initWindow() {
                platform_ = ui::makePlatform();
                window_ = platform_->makeWindow(500, 500);
                window_->setTitle("Spargel Engine - Demo UIRenderer");
                window_->setDelegate(this);
                window_->setAnimating(true);
                window_->bindRenderer(renderer_.get());
            }
            void prepareScene() {
                scene_.setRenderer(renderer_.get());
                scene_.setScale(window_->scaleFactor());
                scene_.setClip(150, 150, 100, 90);
                scene_.fillCircle(150, 150, 50 * 1.4143f, 0xFFFF0000);
                scene_.strokeLine(100, 100, 200, 100, 0xFFFFFFFF);
                scene_.strokeLine(200, 100, 200, 200, 0xFF00FFFF);
                scene_.strokeLine(200, 200, 100, 200, 0xFFFF00FF);
                scene_.strokeLine(100, 200, 100, 100, 0xFFFFFF00);
                scene_.clearClip();
                scene_.strokeCircle(150, 150, 50, 0xFF0000FF);
                scene_.fillText(text::StyledText{"hello,world测试日本語"_sv, font_.get()}, 20, 250,
                                0xFF0000FF);
                scene_.strokeLine(0, 250, 500, 250, 0xFFFFFFFF);
                scene_.dump();
            }

            base::UniquePtr<ui::Platform> platform_;
            base::UniquePtr<ui::Window> window_;
            base::UniquePtr<gpu::GPUContext> gpu_context_;
            base::UniquePtr<render::UIRenderer> renderer_;
            base::UniquePtr<text::TextShaper> shaper_;
            base::UniquePtr<text::Font> font_;
            render::UIScene scene_;
        };
    }  // namespace
}  // namespace spargel::render

int main() {
    spargel::render::Demo demo;
    demo.start();
    return 0;
}
