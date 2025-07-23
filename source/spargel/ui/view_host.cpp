#include "spargel/base/logging.h"
#include "spargel/render/ui_renderer.h"
#include "spargel/ui/view.h"
#include "spargel/ui/view_host.h"

namespace spargel::ui {
    ViewHost::ViewHost(Window* window, render::UIRenderer* renderer)
        : window_{window}, renderer_{renderer} {
        window_->setDelegate(this);
        window_->setAnimating(false);
        window_->bindRenderer(renderer_);
        scene_.setScale(window_->scaleFactor());
        scene_.setRenderer(renderer_);
    }
    void ViewHost::setRootView(View* view) {
        root_view_ = view;
        root_view_->setHost(this);
        setDirty();
    }
    void ViewHost::setDirty() {
        dirty_ = true;
        window_->requestRedraw();
    }
    void ViewHost::onRender() {
        if (!root_view_) {
            return;
        }
        scene_.clear();
        // TODO: GPUContext.
        PaintContext ctx{nullptr, &scene_};
        root_view_->paint(ctx);
        renderer_->render(scene_);
        dirty_ = false;
    }
    void ViewHost::onMouseDown(float x, float y) {
        if (!root_view_) {
            return;
        }
        root_view_->onMouseDown(x, y);
    }
    text::TextShaper* ViewHost::getTextShaper() {
        return renderer_->getTextShaper();
    }
}  // namespace spargel::ui
