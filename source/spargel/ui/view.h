#pragma once

#include "spargel/base/span.h"
#include "spargel/base/string.h"
#include "spargel/base/unique_ptr.h"
#include "spargel/base/vector.h"
#include "spargel/math/rectangle.h"
#include "spargel/math/vector.h"

namespace spargel::render {
    class UIScene;
}

namespace spargel::ui {

    class ViewHost;

    // A View corresponds to a complete functionality of the UI.
    //
    // The View class should be sub-classed to provide custom behaviour.
    //
    class View {
    public:
        View() {}
        virtual ~View() = default;

        ViewHost* host() { return host_; }
        void setHost(ViewHost* host) { host_ = host; }

        // View Hierarchy
        // --------------
        //
        // Every view owns its children.
        //

        base::Span<View*> children() { return children_.toSpan(); }

        void addChild(View* v);

        View* parent() { return parent_; }
        // This should only be called by the parent.
        void setParent(View* p) { parent_ = p; }

        // Positioning
        // -----------

        // The bounding box in parent's coordinate system.
        math::Rectangle frame() const { return frame_; }

        // The bounding box in the view's coordinate system.
        math::Rectangle bounds() const;

        // Layout
        // ------
        //
        // Every view manages the positions of its children.
        // The layout of the children is determined via a negotiation process.

        // How much space this view needs given the proposal.
        virtual math::RectangleSize preferredSize(math::RectangleSize proposal) {
            return proposal;
        }
        // Put the children at correct places.
        virtual void placeChildren() {}

        // Paint
        // -----

        // Record painting commands to the scene.
        virtual void paint([[maybe_unused]] render::UIScene& scene) {}

        // Ask the ViewHost to schedule a repaint.
        void requestRepaint();

        // Event Handling
        // --------------

        virtual void onMouseDown([[maybe_unused]] float x, [[maybe_unused]] float y) {}

    private:
        ViewHost* host_;
        View* parent_ = nullptr;
        base::vector<View*> children_;

        math::Rectangle frame_;
        // Offset in parent's coordinate.
        math::Vector2f offset_;
    };

    class ButtonView : public View {
    public:
        ButtonView();
    };

}  // namespace spargel::ui
