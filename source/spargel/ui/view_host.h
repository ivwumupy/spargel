#pragma once

#include "spargel/base/unique_ptr.h"
#include "spargel/render/ui_scene.h"
#include "spargel/ui/window.h"

namespace spargel::render {
    class UIRenderer;
}

namespace spargel::text {
    class TextShaper;
}

namespace spargel::ui {
    class View;

    // ViewHost connects a window and a view hierarchy.
    class ViewHost final : public WindowDelegate {
    public:
        ViewHost(Window* window, render::UIRenderer* renderer);

        Window* window() { return window_; }

        void setRootView(View* view);
        View* root_view() { return root_view_; }

        void setDirty();

        void invalidateLayout();

        // Repaint the view tree.
        void scheduleRepaint();

        //
        void onRender() override;
        void onMouseDown(float x, float y) override;

        // TODO
        text::TextShaper* getTextShaper();

    private:
        void computeLayout();

        Window* window_;
        View* root_view_ = nullptr;
        render::UIScene scene_;
        render::UIRenderer* renderer_;
        bool dirty_ = true;
        bool needs_layout_ = true;
    };
}  // namespace spargel::ui
