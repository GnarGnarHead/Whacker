#include "input/serial.hpp"

#include <algorithm>

namespace whacker::input {

PaddleCommand serial_command(const float normalized_position, const float court_height) {
    const float clamped = std::clamp(normalized_position, 0.0f, 1.0f);
    return PaddleCommand {.target_y = clamped * court_height, .intent_velocity = 0.0f};
}

}  // namespace whacker::input

