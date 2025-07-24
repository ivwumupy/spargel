#pragma once

#include "spargel/input/input_device.h"

namespace spargel::input {
    struct ButtonInput;

    enum class KeyboardCode {
#define KEYBOARD_CODE(name) name,
#include "spargel/input/keyboard_code.inc"
    };
    class KeyboardDevice final : public InputDevice {
    public:
        KeyboardDevice() : InputDevice{InputDeviceKind::Keyboard} {}

        ButtonInput* getButton(KeyboardCode code);

    private:
    };
}  // namespace spargel::input
