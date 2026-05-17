#include "input/keyboard.hpp"

#include <algorithm>

namespace whacker::input {

PaddleCommand keyboard_command(
    const bool move_up,
    const bool move_down,
    const float previous_target_y,
    const float court_height,
    const float motion_speed,
    const float dt) {
    const int direction = static_cast<int>(move_down) - static_cast<int>(move_up);
    const float next_target =
        std::clamp(previous_target_y + static_cast<float>(direction) * motion_speed * dt, 0.0f, court_height);
    return PaddleCommand {.target_y = next_target, .intent_velocity = static_cast<float>(direction) * motion_speed};
}

}  // namespace whacker::input

