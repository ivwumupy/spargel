#include "spargel/base/unique_ptr.h"
#include "spargel/gpu/gpu_context.h"
#include "spargel/render/ui_renderer.h"
#include "spargel/render/ui_scene.h"
#include "spargel/text/font_manager.h"
#include "spargel/text/text_shaper.h"
#include "spargel/ui/platform.h"
#include "spargel/ui/view.h"
#include "spargel/ui/view_host.h"
#include "spargel/ui/window.h"

namespace spargel::ui {
    namespace {
        using namespace base::literals;

        class TextView : public ui::View {
        public:
            TextView(text::Font* font) : text_{""_sv, font} {}

            void setText(base::StringView s) {
                text_.setText(s);
                invalidateLayout();
                requestRepaint();
            }
            void onPaint(render::UIScene& scene) override {
                // TODO: Move to layout.
                auto shaper = getHost()->getTextShaper();
                auto shape_result = shaper->shapeLine(text_);
                spargel_log_info("shape done: width = %.3f", shape_result.width);

                // TODO: Transform to window coordinate.
                scene.strokeRectangle(getFrame(), 0xFFFF00FF);

                auto frame = getFrame();

                float x = frame.origin.x + frame.size.width / 2.0f - shape_result.width / 2.0f - shape_result.leading;
                float y = frame.origin.y + frame.size.height / 2.0f + shape_result.ascent / 2.0f + shape_result.descent / 2.0f;
                scene.fillText(text_, x, y, 0xFFFFFFFF);
            }

        private:
            text::StyledText text_;
        };

        class DemoView : public ui::View {
        public:
            DemoView(text::FontManager* font_manager) {
                font_ = font_manager->createDefaultFont();
                text_ = emplaceChild<TextView>(font_.get());
                text_->setText("<test>hello"_sv);
                setFrame(0, 0, 200, 200);
                text_->setFrame(50, 50, 100, 100);
            }

            void onPaint(render::UIScene& scene) override {
                scene.setClip(getFrame());
                scene.strokeCircle(250, 250, 100, state_ ? 0xFF00FF00 : 0xFFFF0000);
            }

            void onMouseDown([[maybe_unused]] float x, [[maybe_unused]] float y) override {
                state_ = !state_;
                requestRepaint();
            }

        private:
            bool state_ = false;
            base::UniquePtr<text::Font> font_;
            TextView* text_;
        };

        class DemoApp {
        public:
            DemoApp() {
                platform_ = ui::makePlatform();
                window_ = platform_->makeWindow(500, 500);
                window_->setTitle("Spargel Engine - UI Demo");
                gpu_context_ = gpu::makeContext(gpu::inferBackend());
                font_manager_ = text::FontManager::create();
                shaper_ = text::TextShaper::create(font_manager_.get());
                renderer_ = render::makeUIRenderer(gpu_context_.get(), shaper_.get());
                view_host_ = base::makeUnique<ui::ViewHost>(window_.get(), renderer_.get());
                root_view_ = base::makeUnique<DemoView>(font_manager_.get());
                view_host_->setRootView(root_view_.get());
            }

            void start() { platform_->startLoop(); }

        private:
            base::UniquePtr<ui::Platform> platform_;
            base::UniquePtr<ui::Window> window_;
            base::UniquePtr<gpu::GPUContext> gpu_context_;
            base::UniquePtr<text::FontManager> font_manager_;
            base::UniquePtr<text::TextShaper> shaper_;
            base::UniquePtr<render::UIRenderer> renderer_;
            base::UniquePtr<ui::ViewHost> view_host_;
            base::UniquePtr<View> root_view_;
        };
    }  // namespace
}  // namespace spargel::ui

int main() {
    spargel::ui::DemoApp{}.start();
    return 0;
}
