#include "spargel/base/string_view.h"
#include "spargel/base/unique_ptr.h"
#include "spargel/gpu/gpu_context.h"
#include "spargel/math/function.h"
#include "spargel/render/ui_renderer.h"
#include "spargel/render/ui_scene.h"
#include "spargel/text/font.h"
#include "spargel/text/font_manager.h"
#include "spargel/text/styled_text.h"
#include "spargel/text/text_shaper.h"
#include "spargel/ui/platform.h"
#include "spargel/ui/window.h"

namespace spargel::render {
    namespace {
        using namespace base::literals;

        class Demo final : public ui::WindowDelegate {
        public:
            Demo() {
                font_manager_ = text::FontManager::create();
                font_ = font_manager_->defaultFont();
                initRenderer();
                initWindow();
                prepareScene();
            }
            void start() { platform_->startLoop(); }
            void onRender() override {
                frame_count_++;
                scene_.clear();
                prepareScene();
                renderer_->render(scene_);
            }

        private:
            void initRenderer() {
                gpu_context_ = gpu::makeContext(gpu::inferBackend());
                shaper_ = text::TextShaper::create(font_manager_.get());
                renderer_ = render::makeUIRenderer(gpu_context_.get(), shaper_.get());
            }
            void initWindow() {
                platform_ = ui::makePlatform();
                window_ = platform_->makeWindow(1500, 1000);
                window_->setTitle("Spargel Engine - Demo UIRenderer");
                window_->setDelegate(this);
                window_->setAnimating(true);
                window_->bindRenderer(renderer_.get());
            }
            void prepareScene() {
                scene_.setRenderer(renderer_.get());
                scene_.setScale(window_->scaleFactor());
                scene_.setClip(0, 0, 1500, 1000);
                scene_.fillCircle(150, 150, 50 * 1.4143f, 0xFFFF0000);
                scene_.strokeLine(100, 100, 200, 100, 0xFFFFFFFF);
                scene_.strokeLine(200, 100, 200, 200, 0xFF00FFFF);
                scene_.strokeLine(200, 200, 100, 200, 0xFFFF00FF);
                scene_.strokeLine(100, 200, 100, 100, 0xFFFFFF00);
                scene_.strokeCircle(150, 150, 50, 0xFF0000FF);
                scene_.fillText(text::StyledText{"hello,world测试日本語"_sv, font_}, 20, 250,
                                0xFF0000FF);
                scene_.strokeLine(0, 250, 500, 250, 0xFFFFFFFF);

                // move on a circle!
                float x = 520.0f + 30.0f * math::cos((float)frame_count_ / 120.0f);
                float y = 250.0f + 30.0f * math::sin((float)frame_count_ / 120.0f);
                scene_.fillText(text::StyledText{"hello,world测试日本語"_sv, font_}, x, y,
                                0xFF0000FF);

                scene_.fillText(text::StyledText{"hello,world测试日本語"_sv, font_}, 720, 750,
                                0xFF0000FF);

                scene_.fillText(text::StyledText{"hello,worldकैसे हैं आप测试日本語"_sv, font_}, 20, 750,
                                0xFF0000FF);

                scene_.fillRoundedRect(200 - 5, 400, 100 + 10, 50, 5, 0xFFFFFFFF);
                scene_.fillText(text::StyledText{"click"_sv, font_}, 202, 442, 0xFF000000);

                // slide (width = 120)
                float p = (frame_count_ % 1200) / 6.0f;
                p = 100.0f - math::abs(100.0f - p);
                scene_.fillRoundedRect(400, 425, 120.0f * p / 100.0f, 10, 5, 0xFFFF0000);
                scene_.fillRoundedRect(400 + 120.0f * p / 100.0f, 425,
                                       120.0f * (100.0f - p) / 100.0f, 10, 5, 0xFFCCCCCC);
                scene_.fillCircle(400 + 120.0f * p / 100.0f, 425 + 5, 8, 0xFF0055FF);

                scene_.fillText(text::StyledText{"slide"_sv, font_}, 410, 410, 0xFF777777);

                scene_.strokeLine(50 + 6, 50, 50, 50 + 6, 0xFFFFFFFF);
                scene_.strokeLine(50, 50, 50 + 6, 50 + 6, 0xFFFFFFFF);

                scene_.dump();
            }

            base::UniquePtr<ui::Platform> platform_;
            base::UniquePtr<ui::Window> window_;
            base::UniquePtr<gpu::GPUContext> gpu_context_;
            base::UniquePtr<render::UIRenderer> renderer_;
            base::UniquePtr<text::FontManager> font_manager_;
            base::UniquePtr<text::TextShaper> shaper_;
            text::Font* font_;
            render::UIScene scene_;
            u32 frame_count_ = 0;
        };
    }  // namespace
}  // namespace spargel::render

int main() {
    spargel::render::Demo demo;
    demo.start();
    return 0;
}
