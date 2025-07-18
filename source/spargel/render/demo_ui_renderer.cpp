#include <spargel/base/unique_ptr.h>
#include <spargel/gpu/gpu_context.h>
#include <spargel/render/ui_renderer.h>
#include <spargel/render/ui_scene.h>
#include <spargel/resource/directory.h>
#include <spargel/resource/resource.h>
#include <spargel/ui/platform.h>
#include <spargel/ui/view_host.h>
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
                // Use resource manager to load shaders.
                // Assumed filesystem:
                // build/
                //   bin/
                //     demo_ui_renderer
                //   source/spargel/render/resources
                //     shaders
                //
                // TODO: Improve
                resource_manager_ = resource::makeRelativeManager("..");
                initRenderer();
                initWindow();
                prepareScene();
            }
            void start() { platform_->startLoop(); }
            void onRender() override { renderer_->render(scene_); }

        private:
            void initRenderer() {
                gpu_context_ = gpu::makeContext(inferBackend());
                renderer_ = render::makeUIRenderer(gpu_context_.get(), resource_manager_.get());
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
                scene_.setClip(150, 150, 100, 100);
                scene_.fillCircle(150, 150, 50 * 1.4143, 0xFFFF0000);
                scene_.strokeLine(100, 100, 200, 100, 0xFFFFFFFF);
                scene_.strokeLine(200, 100, 200, 200, 0xFF00FFFF);
                scene_.strokeLine(200, 200, 100, 200, 0xFFFF00FF);
                scene_.strokeLine(100, 200, 100, 100, 0xFFFFFF00);
                scene_.clearClip();
                scene_.strokeCircle(150, 150, 50, 0xFF0000FF);
            }

            base::UniquePtr<resource::ResourceManager> resource_manager_;
            base::UniquePtr<ui::Platform> platform_;
            base::UniquePtr<ui::Window> window_;
            base::UniquePtr<gpu::GPUContext> gpu_context_;
            base::UniquePtr<render::UIRenderer> renderer_;
            render::UIScene scene_;
        };
    }  // namespace
}  // namespace spargel::render

int main() {
    spargel::render::Demo demo;
    demo.start();
    return 0;
}
