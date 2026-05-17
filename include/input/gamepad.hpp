#pragma once

#include "input/types.hpp"

namespace whacker::input {

PaddleCommand gamepad_command(float stick_y, float previous_target_y, float court_height, float speed, float dt);

}  // namespace whacker::input

