#include "spargel/base/optional.h"
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
            ~TextView() { spargel_log_info("destructed"); }

            void setText(base::StringView s) {
                text_.setText(s);
                shape_result_ = base::nullopt;
                invalidateLayout();
                requestRepaint();
            }
            math::RectangleSize getPreferredSize(
                [[maybe_unused]] math::RectangleSize proposal) override {
                if (!shape_result_) {
                    shape();
                }
                return {shape_result_->width, shape_result_->ascent - shape_result_->descent};
            }
            void onPaint(render::UIScene& scene) override {
                scene.strokeRectangle(0, 0, width(), height(), 0xFFFF00FF);

                if (!shape_result_) {
                    shape();
                }

                float x = width() / 2.0f - shape_result_->width / 2.0f - shape_result_->leading;
                float y =
                    height() / 2.0f + shape_result_->ascent / 2.0f + shape_result_->descent / 2.0f;
                scene.fillText(text_, x, y, 0xFFFFFFFF);
            }

        private:
            void shape() {
                auto shaper = getHost()->getTextShaper();
                shape_result_ = shaper->shapeLine(text_);
                spargel_log_info("shape done: width = %.3f", shape_result_->width);
            }

            text::StyledText text_;
            base::Optional<text::ShapedLine> shape_result_;
        };

        class BoxView : public ui::View {
        public:
            BoxView(u32 color) : color_{color} {}

            void onPaint(render::UIScene& scene) override {
                scene.setClip(0, 0, 500, 500);
                scene.strokeCircle(250, 250, 100, color_);
            }

        private:
            u32 color_;
        };

        class FillLayout : public ui::View {
        public:
            void setChild(ui::View* child) {
                if (child_) {
                    auto old = removeChild(0);
                    delete old;
                }
                child_ = child;
                addChild(child_);
            }
            // NOTE: This is the default implementation. So there is no need for this view.
            math::RectangleSize getPreferredSize(math::RectangleSize proposal) override {
                return proposal;
            }
            void placeChildren() override {
                spargel_check(getChildren().count() == 1);
                child_->setFrame(getFrame());
            }

        private:
            ui::View* child_ = nullptr;
        };

        class HStackLayout : public ui::View {
        public:
            math::RectangleSize getPreferredSize(math::RectangleSize proposal) override {
                return proposal;
            }
            void placeChildren() override {}
        };

        class DemoView : public ui::View {
        public:
            DemoView(text::FontManager* font_manager) {
                font_ = font_manager->defaultFont();
                // text_ = emplaceChild<TextView>(font_.get());
                text_ = new TextView(font_);
                text_->setText("<test>hello"_sv);
                auto fill = emplaceChild<FillLayout>();
                fill->addChild(text_);
            }
            ~DemoView() { spargel_log_info("destructed"); }

            void placeChildren() override {
                spargel_log_info("%.3f %.3f", getFrame().size.width, getFrame().size.height);
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
            text::Font* font_;
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
                // root_view_ = base::makeUnique<BoxView>(0xFF0000FF);
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
