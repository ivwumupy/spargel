#include "spargel/base/check.h"
#include "spargel/ui/view.h"
#include "spargel/ui/view_host.h"

namespace spargel::ui {
    void View::addChild(View* v) {
        spargel_check(!v->getParent());
        children_.emplace(v);
        v->setParent(this);
        v->setHost(host_);
    }
    void View::requestRepaint() { host_->setDirty(); }
    void View::paint(PaintContext& context) {
        // TODO: Support alternative path.
        onPaint(*context.scene);
        for (auto* child : children_) {
            child->paint(context);
        }
    }
    void View::invalidateLayout() {
        needs_layout_ = true;
        if (parent_) {
            // If this view needs to recompute layoute, then the parent must.
            parent_->invalidateLayout();
        } else {
            // There is no parent.
            // In this case, notify the host.
            spargel_check(host_ != nullptr);
            host_->invalidateLayout();
        }
    }
}  // namespace spargel::ui
