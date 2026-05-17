#pragma once

#include "input/types.hpp"

namespace whacker::input {

PaddleCommand keyboard_command(
    bool move_up,
    bool move_down,
    float previous_target_y,
    float court_height,
    float motion_speed,
    float dt);

}  // namespace whacker::input

