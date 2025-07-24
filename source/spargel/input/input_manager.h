#pragma once

#include "spargel/base/vector.h"

namespace spargel::input {
    class InputDevice;
    class KeyboardDevice;
    class InputManager {
    public:
        InputManager();

        // Find a keyboard device.
        KeyboardDevice* findKeyboard();

    private:
        base::Vector<InputDevice*> devices_;
    };
}  // namespace spargel::input
