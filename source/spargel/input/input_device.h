#pragma once

namespace spargel::input {
    enum class InputDeviceKind {
        Mouse,
        Keyboard,
        Controller,
        RacingWheel,
        Unknown,
    };
    class InputDevice {
    public:
        virtual ~InputDevice() = default;

        InputDeviceKind kind() const { return kind_; }

    protected:
        explicit InputDevice(InputDeviceKind kind) : kind_{kind} {}

    private:
        InputDeviceKind kind_;
    };
}  // namespace spargel::input
