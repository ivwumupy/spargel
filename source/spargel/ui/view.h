#pragma once

#include "spargel/base/span.h"
#include "spargel/base/string.h"
#include "spargel/base/unique_ptr.h"
#include "spargel/base/vector.h"
#include "spargel/math/rectangle.h"
#include "spargel/math/vector.h"

namespace spargel::gpu {
    class GPUContext;
}

namespace spargel::render {
    class UIScene;
}

namespace spargel::ui {

    class ViewHost;

    struct PaintContext {
        gpu::GPUContext* gpu_context;
        render::UIScene* scene;
    };

    // A View corresponds to a complete functionality of the UI.
    //
    // The View class should be sub-classed to provide custom behaviour.
    //
    class View {
    public:
        View() {}
        virtual ~View() = default;

        // Host is set for every view in the hierarchy.
        ViewHost* getHost() { return host_; }
        void setHost(ViewHost* host) { host_ = host; }

        bool isRootView() const { return parent_ == nullptr; }

        // View Hierarchy
        // --------------
        //
        // Every view owns its children.
        //

        base::Span<View*> getChildren() { return children_.toSpan(); }

        void addChild(View* v);

        View* getParent() { return parent_; }
        // This should only be called by the parent.
        void setParent(View* p) { parent_ = p; }

        // Positioning
        // -----------

        // The bounding box in parent's coordinate system.
        math::Rectangle getFrame() const { return frame_; }

        float getWidth() const;
        float getHeight() const;

        // Layout
        // ------
        //
        // Every view manages the positions of its children.
        // The layout of the children is determined via a negotiation process.

        // How much space this view needs given the proposal.
        virtual math::RectangleSize getPreferredSize(math::RectangleSize proposal) {
            return proposal;
        }
        // Put the children at correct places.
        virtual void placeChildren() {}

        void invalidateLayout();

        bool needsLayout() const { return needs_layout_; }

        // Paint
        // -----

        // Paint the view.
        // NOTE: This is called by the framework.
        void paint(PaintContext& context);

        // Ask the ViewHost to schedule a repaint.
        void requestRepaint();

        // Event Handling
        // --------------
        //

        virtual void onMouseDown([[maybe_unused]] float x, [[maybe_unused]] float y) {}

        // Record painting commands to the scene.
        virtual void onPaint([[maybe_unused]] render::UIScene& scene) {}

        // Bypass UIRenderer.
        // virtual void onPaint([[maybe_unused]] gpu::GPUContext& ctx) {}

    private:
        ViewHost* host_;
        View* parent_ = nullptr;
        base::vector<View*> children_;

        math::Rectangle frame_;
        // Offset in parent's coordinate.
        math::Vector2f offset_;

        bool needs_layout_ = true;
    };

    class ButtonView : public View {
    public:
        ButtonView();
    };

}  // namespace spargel::ui
