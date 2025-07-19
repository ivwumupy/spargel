#include <spargel/base/unique_ptr.h>
#include <spargel/config.h>
#include <spargel/gpu/gpu_context.h>
#include <spargel/render/ui_renderer.h>
#include <spargel/render/ui_scene.h>
#include <spargel/text/text_shaper.h>
#include <spargel/ui/platform.h>
#include <spargel/ui/view.h>
#include <spargel/ui/view_host.h>
#include <spargel/ui/window.h>

namespace spargel::editor {
    class EditorView : public ui::View {
    public:
        void paint(render::UIScene& scene) override {
            scene.strokeCircle(250, 250, 100, state_ ? 0xFF00FF00 : 0xFFFF0000);
        }

        void onMouseDown([[maybe_unused]] float x, [[maybe_unused]] float y) override {
            state_ = !state_;
            reportDirty();
        }

    private:
        bool state_ = false;
    };

    gpu::GPUBackend inferBackend() {
#if SPARGEL_IS_MACOS
        return gpu::GPUBackend::metal;
#endif
        spargel_panic("cannot determine backend");
    }

    class Editor {
    public:
        Editor() {
            platform_ = ui::makePlatform();
            window_ = platform_->makeWindow(500, 500);
            window_->setTitle("Spargel Engine - Editor");
            gpu_context_ = gpu::makeContext(inferBackend());
            shaper_ = text::TextShaper::create();
            renderer_ = render::makeUIRenderer(gpu_context_.get(), shaper_.get());
            view_host_ = base::makeUnique<ui::ViewHost>(window_.get(), renderer_.get());
            root_view_ = base::makeUnique<EditorView>();
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
        base::UniquePtr<EditorView> root_view_;
    };
}  // namespace spargel::editor

int main() {
    spargel::editor::Editor{}.start();
    return 0;
}
