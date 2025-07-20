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

    struct ProposedSize {
        // TODO: We need to represent: a finite value, or "infinity"
        float width;
        float height;
    };

    struct PreferredSize {};

    // A LayoutManager is owned by a View. It manages how the View's children should be laid out
    // within the View's content bounds.
    class LayoutManager {
    public:
        virtual ~LayoutManager() = default;

        // TODO: What's the return type?
        virtual void preferredSize(ProposedSize proposal) = 0;
        virtual void placeChildren() = 0;
    };

    // A View corresponds to a complete functionality of the UI.
    //
    // The View class should be sub-classed to provide custom behaviour.
    //
    class View {
    public:
        View() {}
        virtual ~View() = default;

        void setHost(ViewHost* host) { host_ = host; }

        // View Hierarchy
        // --------------

        base::Span<View*> children() { return children_.toSpan(); }

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

        virtual void paint([[maybe_unused]] render::UIScene& scene) {}

        void requestRepaint();

        // Event Handling
        // --------------

        virtual void onMouseDown([[maybe_unused]] float x, [[maybe_unused]] float y) {}

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
