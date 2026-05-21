#pragma once

#include <array>
#include <cstdint>

namespace whacker::app {

enum class InputAction : std::uint8_t {
    MenuUp = 0,
    MenuDown,
    MenuLeft,
    MenuRight,
    Confirm,
    Back,
    Pause,
    Count
};

constexpr int kInputActionCount = static_cast<int>(InputAction::Count);

struct KeyboardPhysicalState {
    bool key_up = false;
    bool key_down = false;
    bool key_left = false;
    bool key_right = false;
    bool key_w = false;
    bool key_s = false;
    bool key_enter = false;
    bool key_kp_enter = false;
    bool key_space = false;
    bool key_escape = false;
};

struct ActionInputFrame {
    std::array<bool, kInputActionCount> held {};
    std::array<bool, kInputActionCount> pressed {};
    std::array<bool, kInputActionCount> released {};
    float p1_move_y = 0.0f;
    float p2_move_y = 0.0f;
};

ActionInputFrame derive_keyboard_action_frame(
    const KeyboardPhysicalState& previous,
    const KeyboardPhysicalState& current);

bool input_held(const ActionInputFrame& frame, InputAction action);
bool input_pressed(const ActionInputFrame& frame, InputAction action);
bool input_released(const ActionInputFrame& frame, InputAction action);

}  // namespace whacker::app
