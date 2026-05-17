#include "input/gamepad.hpp"

#include <algorithm>

namespace whacker::input {

PaddleCommand gamepad_command(
    const float stick_y,
    const float previous_target_y,
    const float court_height,
    const float speed,
    const float dt) {
    const float axis = std::clamp(stick_y, -1.0f, 1.0f);
    const float target = std::clamp(previous_target_y + axis * speed * dt, 0.0f, court_height);
    return PaddleCommand {.target_y = target, .intent_velocity = axis * speed};
}

}  // namespace whacker::input

