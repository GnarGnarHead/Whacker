#pragma once

#include "input/types.hpp"

namespace whacker::input {

PaddleCommand serial_command(float normalized_position, float court_height);

}  // namespace whacker::input

