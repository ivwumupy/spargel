#include <spargel/base/unique_ptr.h>
#include <spargel/gpu/gpu_context.h>
#include <spargel/render/ui_renderer.h>
#include <spargel/ui/platform.h>
#include <spargel/ui/window.h>

//
#include <stdio.h>

namespace spargel::render {
    namespace {
        gpu::GPUBackend inferBackend() {
#if SPARGEL_IS_MACOS
            return gpu::GPUBackend::metal;
#endif
            spargel_panic("cannot determine backend");
        }
        class Demo final : public ui::WindowDelegate {
        public:
            Demo() {
                // Create renderer.
                gpu_context_ = gpu::makeContext(inferBackend());
                renderer_ = render::makeUIRenderer(gpu_context_.get());

                // Create window.
                platform_ = ui::makePlatform();
                window_ = platform_->makeWindow(500, 500);
                window_->setTitle("Spargel Engine - Demo UIRenderer");
                window_->setDelegate(this);
                window_->setAnimating(true);
                window_->bindRenderer(renderer_.get());
            }
            void start() {
                platform_->startLoop();
            }
            void onRender() override {
                // TODO: Build scene.
                renderer_->render();
            }

        private:
            base::UniquePtr<ui::Platform> platform_;
            base::UniquePtr<ui::Window> window_;
            base::UniquePtr<gpu::GPUContext> gpu_context_;
            base::UniquePtr<render::UIRenderer> renderer_;
        };
    }  // namespace
}  // namespace spargel::render

int main() {
    spargel::render::Demo demo;
    demo.start();
    return 0;
}
