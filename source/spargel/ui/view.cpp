#include "spargel/ui/view.h"

#include "spargel/base/check.h"
#include "spargel/ui/view_host.h"

namespace spargel::ui {
    View::~View() {
        for (auto* child : children_) {
            delete child;
        }
    }
    void View::setHost(ViewHost* host) {
        host_ = host;
        for (auto* child : getChildren()) {
            child->setHost(host);
        }
    }
    void View::addChild(View* v) {
        spargel_check(!v->getParent());
        children_.emplace(v);
        v->setParent(this);
        v->setHost(host_);
    }
    View* View::removeChild(usize i) {
        auto child = children_[i];
        // TODO: Children order is not stable?
        children_.eraseFast(i);
        return child;
    }
    void View::requestRepaint() {
        if (host_) {
            host_->setDirty();
        }
    }
    void View::paint(PaintContext& context) {
        // TODO: Support alternative path.
        // TODO: Should we put the transform stack in the paint context.
        context.scene->pushTransform(frame_.origin.x, frame_.origin.y);
        onPaint(*context.scene);
        for (auto* child : children_) {
            child->paint(context);
        }
        context.scene->popTransform();
    }
    void View::invalidateLayout() {
        needs_layout_ = true;
        if (parent_) {
            // If this view needs to recompute layoute, then the parent must.
            parent_->invalidateLayout();
        } else {
            // There is no parent.
            // In this case, notify the host.
            // NOTE: It's possible that the view has not been attached to a host.
            if (host_) {
                host_->invalidateLayout();
            }
        }
    }
}  // namespace spargel::ui
