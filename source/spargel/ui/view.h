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
    class ViewHost;

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
        View() {}
        virtual ~View() = default;

        void setHost(ViewHost* host) { host_ = host; }

        // View Hierarchy
        // --------------
        //

        base::Span<View*> children();

        void addChild(View* v);

        View* getParent() { return parent_; }
        void setParent(View* v) { parent_ = v; }

        // Layout
        // ------

        void setLayoutManager(LayoutManager* manager);
        LayoutManager& getLayoutManager() { return *layout_manager_.get(); }

        /// Layout all the children.
        void layout();

        /// Get the preferred size of the view.
        void getPreferredSize();

        // Paint
        // -----

        virtual void paint(render::UIScene& scene) {}

        void reportDirty();

        // Event Handling
        // --------------

        virtual void onMouseDown(float x, float y) {}

    private:
        ViewHost* host_;
        View* parent_ = nullptr;
        base::vector<View*> children_;
        base::unique_ptr<LayoutManager> layout_manager_;
    };

    class ButtonView : public View {
    public:
        ButtonView();
    };

}  // namespace spargel::ui
