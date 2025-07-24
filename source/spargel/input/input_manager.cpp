#include "spargel/input/input_manager.h"

#include "spargel/input/keyboard_device.h"

namespace spargel::input {
    InputManager::InputManager() { devices_.push(new KeyboardDevice); }
    KeyboardDevice* InputManager::findKeyboard() {
        for (auto device : devices_) {
            if (device->kind() == InputDeviceKind::Keyboard) {
                return static_cast<KeyboardDevice*>(device);
            }
        }
        return nullptr;
    }
}  // namespace spargel::input
