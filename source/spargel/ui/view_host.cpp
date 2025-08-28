#include "spargel/ui/view_host.h"

#include "spargel/render/ui_renderer.h"
#include "spargel/ui/view.h"

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

        if (needs_layout_) {
            computeLayout();
        }

        scene_.clear();
        // TODO: GPUContext.
        PaintContext ctx{nullptr, &scene_};
        root_view_->paint(ctx);
        renderer_->render(scene_);
        dirty_ = false;
    }
    void ViewHost::onMouseDown(MouseDownEvent const& e) {
        if (!root_view_) {
            return;
        }
        // root_view_->onMouseDown(e.x, e.y);
        root_view_->handleMousePressed(e);
    }
    void ViewHost::invalidateLayout() {
        needs_layout_ = true;
        window_->requestRedraw();
    }
    void ViewHost::computeLayout() {
        spargel_check(root_view_);
        root_view_->setFrame(0, 0, window_->width(), window_->height());
        root_view_->placeChildren();
    }
}  // namespace spargel::ui
