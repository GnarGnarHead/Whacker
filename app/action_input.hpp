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
constexpr int kMaxInputControllers = 4;
constexpr int kKeyboardScancodeCount = 512;
constexpr int kKeyboardScancodeUnbound = -1;
constexpr int kKeyboardScancodeW = 26;
constexpr int kKeyboardScancodeS = 22;
constexpr int kKeyboardScancodeUp = 82;
constexpr int kKeyboardScancodeDown = 81;
constexpr int kKeyboardScancodeLeft = 80;
constexpr int kKeyboardScancodeRight = 79;
constexpr int kKeyboardScancodeReturn = 40;
constexpr int kKeyboardScancodeKpEnter = 88;
constexpr int kKeyboardScancodeSpace = 44;
constexpr int kKeyboardScancodeEscape = 41;

enum class PlayerSlot : std::uint8_t {
    P1,
    P2
};

enum class ControllerAxis : std::uint8_t {
    LeftX = 0,
    LeftY,
    RightX,
    RightY,
    Count
};

enum class ControllerButton : std::uint8_t {
    A = 0,
    B,
    X,
    Y,
    Back,
    Guide,
    Start,
    LeftStick,
    RightStick,
    LeftShoulder,
    RightShoulder,
    DpadUp,
    DpadDown,
    DpadLeft,
    DpadRight,
    Count,
    Unbound = 255
};

constexpr int kControllerAxisCount = static_cast<int>(ControllerAxis::Count);
constexpr int kControllerButtonCount = static_cast<int>(ControllerButton::Count);

struct KeyboardPhysicalState {
    std::array<bool, kKeyboardScancodeCount> scancodes {};
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

struct ControllerPhysicalState {
    bool connected = false;
    std::array<float, kControllerAxisCount> axes {};
    std::array<bool, kControllerButtonCount> buttons {};
};

struct InputPhysicalState {
    KeyboardPhysicalState keyboard {};
    std::array<ControllerPhysicalState, kMaxInputControllers> controllers {};
};

struct ControllerPlayerBinding {
    int controller_index = 0;
    ControllerAxis move_y_axis = ControllerAxis::LeftY;
    bool invert_move_y_axis = false;
    ControllerButton move_up_button = ControllerButton::DpadUp;
    ControllerButton move_down_button = ControllerButton::DpadDown;
};

struct ActionInputBindings {
    float controller_axis_deadzone = 0.35f;
    int menu_up_key = kKeyboardScancodeUp;
    int menu_down_key = kKeyboardScancodeDown;
    int menu_secondary_up_key = kKeyboardScancodeW;
    int menu_secondary_down_key = kKeyboardScancodeS;
    int menu_left_key = kKeyboardScancodeLeft;
    int menu_right_key = kKeyboardScancodeRight;
    int menu_confirm_key = kKeyboardScancodeReturn;
    int menu_secondary_confirm_key = kKeyboardScancodeSpace;
    int menu_tertiary_confirm_key = kKeyboardScancodeKpEnter;
    int menu_back_key = kKeyboardScancodeEscape;
    int pause_key = kKeyboardScancodeEscape;
    int menu_controller_index = 0;
    ControllerAxis menu_x_axis = ControllerAxis::LeftX;
    ControllerAxis menu_y_axis = ControllerAxis::LeftY;
    ControllerButton menu_up_button = ControllerButton::DpadUp;
    ControllerButton menu_down_button = ControllerButton::DpadDown;
    ControllerButton menu_left_button = ControllerButton::DpadLeft;
    ControllerButton menu_right_button = ControllerButton::DpadRight;
    ControllerButton menu_confirm_button = ControllerButton::A;
    ControllerButton menu_back_button = ControllerButton::B;
    ControllerButton menu_secondary_back_button = ControllerButton::Back;
    ControllerButton pause_button = ControllerButton::Start;
    int p1_move_up_key = kKeyboardScancodeW;
    int p1_move_down_key = kKeyboardScancodeS;
    int p2_move_up_key = kKeyboardScancodeUp;
    int p2_move_down_key = kKeyboardScancodeDown;
    ControllerPlayerBinding p1_controller {};
    ControllerPlayerBinding p2_controller {
        .controller_index = 1,
        .move_y_axis = ControllerAxis::LeftY,
        .invert_move_y_axis = false,
        .move_up_button = ControllerButton::DpadUp,
        .move_down_button = ControllerButton::DpadDown,
    };
};

struct ActionInputFrame {
    std::array<bool, kInputActionCount> held {};
    std::array<bool, kInputActionCount> pressed {};
    std::array<bool, kInputActionCount> released {};
    float p1_move_y = 0.0f;
    float p2_move_y = 0.0f;
};

ActionInputBindings default_action_input_bindings();
void bind_menu_controller(ActionInputBindings& bindings, int controller_index);
void bind_menu_axes(ActionInputBindings& bindings, ControllerAxis x_axis, ControllerAxis y_axis);
void bind_menu_direction_buttons(
    ActionInputBindings& bindings,
    ControllerButton up_button,
    ControllerButton down_button,
    ControllerButton left_button,
    ControllerButton right_button);
void bind_player_controller(ActionInputBindings& bindings, PlayerSlot player, int controller_index);
void bind_player_move_axis(
    ActionInputBindings& bindings,
    PlayerSlot player,
    ControllerAxis axis,
    bool invert_axis);
void bind_player_move_buttons(
    ActionInputBindings& bindings,
    PlayerSlot player,
    ControllerButton up_button,
    ControllerButton down_button);
bool keyboard_scancode_bindable(int scancode);
bool keyboard_scancode_down(const KeyboardPhysicalState& state, int scancode);

ActionInputFrame derive_action_input_frame(
    const InputPhysicalState& previous,
    const InputPhysicalState& current);

ActionInputFrame derive_action_input_frame(
    const InputPhysicalState& previous,
    const InputPhysicalState& current,
    const ActionInputBindings& bindings);

ActionInputFrame derive_keyboard_action_frame(
    const KeyboardPhysicalState& previous,
    const KeyboardPhysicalState& current);

bool input_held(const ActionInputFrame& frame, InputAction action);
bool input_pressed(const ActionInputFrame& frame, InputAction action);
bool input_released(const ActionInputFrame& frame, InputAction action);

}  // namespace whacker::app
