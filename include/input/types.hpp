#pragma once

#include <cstdint>

namespace whacker::input {

enum class ControlMode : std::uint8_t {
    Keyboard,
    Mouse,
    Gamepad,
    Serial,
    AI
};

struct PaddleCommand {
    float target_y = 0.0f;
    float intent_velocity = 0.0f;
};

}  // namespace whacker::input
