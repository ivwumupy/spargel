#pragma once

#include "spargel/base/span.h"
#include "spargel/base/string.h"
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

    class View;
    class ViewHost;

    struct PaintContext {
        gpu::GPUContext* gpu_context;
        render::UIScene* scene;
    };

    // class SubviewIterator {
    // public:
    //     using ElementType = View*;

    //    explicit SubviewIterator(base::UniquePtr<View>* ptr) : ptr_{ptr} {}

    //    View* operator*() { return ptr_->get(); }

    //    SubviewIterator& operator++() {
    //        ptr_++;
    //        return *this;
    //    }

    //    friend bool operator==(SubviewIterator lhs, SubviewIterator rhs) {
    //        return lhs.ptr_ == rhs.ptr_;
    //    }

    // private:
    //     base::UniquePtr<View>* ptr_;
    // };

    // class SubviewRange {
    // public:
    //     explicit SubviewRange(base::Vector<base::UniquePtr<View>>* subviews) :
    //     subviews_{subviews} {}

    //    SubviewIterator begin() { return SubviewIterator{subviews_->begin()}; }
    //    SubviewIterator end() { return SubviewIterator{subviews_->end()}; }

    // private:
    //     base::Vector<base::UniquePtr<View>>* subviews_;
    // };

    // A View corresponds to a complete functionality of the UI.
    //
    // The View class should be sub-classed to provide custom behaviour.
    //
    class View {
    public:
        View() {}
        virtual ~View();

        // Host is set for every view in the hierarchy.
        ViewHost* getHost() { return host_; }
        void setHost(ViewHost* host);

        bool isRootView() const { return parent_ == nullptr; }

        // View Hierarchy
        // --------------
        //
        // Every view owns its children.
        //

        base::Span<View*> getChildren() { return children_.toSpan(); }

        View* getChild(usize i) { return children_[i]; }

        // The ownership is taken by the view.
        void addChild(View* v);
        template <typename T, typename... Args>
        T* emplaceChild(Args&&... args) {
            auto ptr = new T(base::forward<Args>(args)...);
            addChild(ptr);
            return ptr;
        }

        // The ownership of the removed child is handled over to the caller.
        [[nodiscard]]
        View* removeChild(usize i);

        View* getParent() { return parent_; }
        // This should only be called by the parent.
        void setParent(View* p) { parent_ = p; }

        // Positioning
        // -----------

        // The bounding box in parent's coordinate system.
        math::Rectangle getFrame() const { return frame_; }
        void setFrame(math::Rectangle frame) { frame_ = frame; }
        void setFrame(float x, float y, float w, float h) {
            setFrame(math::Rectangle{{x, y}, {w, h}});
        }

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
