#include "spargel/base/unique_ptr.h"
#include "spargel/gpu/gpu_context.h"
#include "spargel/render/ui_renderer.h"
#include "spargel/render/ui_scene.h"
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
            TextView(text::Font* font) : text_{"<text>"_sv, font} {}

            void setText(text::StyledText text) {
                text_ = text;
                requestRepaint();
            }
            void paint(render::UIScene& scene) override {
                scene.fillText(text_, 100, 100, 0xFFFFFFFF);
            }

        private:
            text::StyledText text_;
        };

        class DemoView : public ui::View {
        public:
            DemoView() {
                font_ = text::createDefaultFont();
                addChild(new TextView(font_.get()));
            }

            void paint(render::UIScene& scene) override {
                scene.strokeCircle(250, 250, 100, state_ ? 0xFF00FF00 : 0xFFFF0000);
                for (auto* child : children()) {
                    child->paint(scene);
                }
            }

            void onMouseDown([[maybe_unused]] float x, [[maybe_unused]] float y) override {
                state_ = !state_;
                requestRepaint();
            }

        private:
            bool state_ = false;
            base::UniquePtr<text::Font> font_;
        };

        class DemoApp {
        public:
            DemoApp() {
                platform_ = ui::makePlatform();
                window_ = platform_->makeWindow(500, 500);
                window_->setTitle("Spargel Engine - UI Demo");
                gpu_context_ = gpu::makeContext(gpu::inferBackend());
                shaper_ = text::TextShaper::create();
                renderer_ = render::makeUIRenderer(gpu_context_.get(), shaper_.get());
                view_host_ = base::makeUnique<ui::ViewHost>(window_.get(), renderer_.get());
                root_view_ = base::makeUnique<DemoView>();
                view_host_->setRootView(root_view_.get());
            }

            void start() { platform_->startLoop(); }

        private:
            base::UniquePtr<ui::Platform> platform_;
            base::UniquePtr<ui::Window> window_;
            base::UniquePtr<gpu::GPUContext> gpu_context_;
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
