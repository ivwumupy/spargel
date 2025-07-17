#pragma once

#include <spargel/base/types.h>
#include <spargel/ui/event.h>

// XKB
#include <xkbcommon/xkbcommon.h>

namespace spargel::ui::xkb {

    struct Context {
        xkb_context* context;
        xkb_keymap* keymap;
        xkb_state* state;

        PhysicalKey translatePhysicalKey(xkb_keycode_t keycode);
    };

}  // namespace spargel::ui::xkb
