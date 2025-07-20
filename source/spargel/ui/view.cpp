#include <spargel/ui/view.h>
#include <spargel/ui/view_host.h>

namespace spargel::ui {
    void View::addChild(View* v) {
        children_.emplace(v);
        v->setParent(this);
        v->setHost(host_);
    }
    void View::requestRepaint() { host_->setDirty(); }
}  // namespace spargel::ui
