#pragma once

#include <spargel/base/span.h>
#include <spargel/base/string.h>
#include <spargel/base/unique_ptr.h>
#include <spargel/base/vector.h>

namespace spargel::render {
    class UIScene;
}

namespace spargel::ui {

    class View;

    /// A LayoutManager is owned by a View. It manages how the View's children should be laid out
    /// within the View's content bounds.
    ///
    class LayoutManager {
    public:
        virtual ~LayoutManager() = default;

        /// Layout the children of |view|.
        virtual void layout(View* view) = 0;
    };

    /// A View corresponds to a complete functionality of the UI.
    ///
    /// The View class should be sub-classed to provide custom behaviour.
    ///
    ///----------------
    /// The Layer Tree
    ///
    /// The `View` hierarchy is backed by a `Layer` tree. Some (but not all) views are layer owning.
    /// Each `Layer` corresponds to a GPU texture.
    ///
    class View {
    public:
        View();
        virtual ~View();

        base::Span<View*> children();

        void addChild(View* v) {
            _children.emplace(v);
            v->setParent(this);
        }

        View* getParent() { return _parent; }
        void setParent(View* v) { _parent = v; }

        void setLayoutManager(LayoutManager* manager);
        LayoutManager& getLayoutManager() { return *_layout_manager.get(); }

        /// Layout all the children.
        void layout();

        /// Get the preferred size of the view.
        void getPreferredSize();

        virtual void paint(render::UIScene& scene) {}

    private:
        View* _parent = nullptr;
        base::vector<View*> _children;
        base::unique_ptr<LayoutManager> _layout_manager;
    };

    class ButtonView : public View {
    public:
        ButtonView();
    };

}  // namespace spargel::ui
