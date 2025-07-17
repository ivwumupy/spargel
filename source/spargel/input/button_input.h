#pragma once

namespace spargel::input {
    enum class ButtonStatus {
        released,
        pressed,
    };
    struct ButtonInput {
        ButtonStatus status;
    };
}  // namespace spargel::input
