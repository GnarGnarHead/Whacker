#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "control_types.hpp"

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
constexpr float kDefaultInputDeadzone = 0.35f;

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

enum class AxisDirection : std::int8_t {
    Negative = -1,
    Positive = 1
};

enum class InputBindingTargetKind : std::uint8_t {
    Action,
    MoveY
};

enum class PhysicalInputKind : std::uint8_t {
    KeyboardScancode,
    ControllerButton,
    ControllerAxis,
    ControllerAxisDirection
};

struct KeyboardPhysicalState {
    std::array<bool, kKeyboardScancodeCount> scancodes {};
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

struct InputBindingTarget {
    InputBindingTargetKind kind = InputBindingTargetKind::Action;
    InputAction action = InputAction::MenuUp;
    InputSlot slot = InputSlot::P1;
};

struct PhysicalInputSource {
    PhysicalInputKind kind = PhysicalInputKind::KeyboardScancode;
    int keyboard_scancode = kKeyboardScancodeUnbound;
    int controller_index = 0;
    ControllerButton controller_button = ControllerButton::Unbound;
    ControllerAxis controller_axis = ControllerAxis::LeftY;
    AxisDirection axis_direction = AxisDirection::Positive;
};

struct ActionInputBinding {
    InputBindingTarget target {};
    PhysicalInputSource source {};
    float output_scale = 1.0f;
    float deadzone = kDefaultInputDeadzone;
};

struct ActionInputBindings {
    std::vector<ActionInputBinding> bindings {};
};

struct ActionInputFrame {
    std::array<bool, kInputActionCount> held {};
    std::array<bool, kInputActionCount> pressed {};
    std::array<bool, kInputActionCount> released {};
    float p1_move_y = 0.0f;
    float p2_move_y = 0.0f;
};

InputBindingTarget action_binding_target(InputAction action);
InputBindingTarget move_y_binding_target(InputSlot slot);
PhysicalInputSource keyboard_scancode_source(int scancode);
PhysicalInputSource controller_button_source(int controller_index, ControllerButton button);
PhysicalInputSource controller_axis_source(int controller_index, ControllerAxis axis);
PhysicalInputSource controller_axis_direction_source(
    int controller_index,
    ControllerAxis axis,
    AxisDirection direction);
void add_action_input_binding(
    ActionInputBindings& bindings,
    InputBindingTarget target,
    PhysicalInputSource source,
    float output_scale = 1.0f,
    float deadzone = kDefaultInputDeadzone);
ActionInputBindings default_action_input_bindings();
void bind_menu_controller(ActionInputBindings& bindings, int controller_index);
void bind_menu_axes(ActionInputBindings& bindings, ControllerAxis x_axis, ControllerAxis y_axis);
void bind_menu_direction_buttons(
    ActionInputBindings& bindings,
    ControllerButton up_button,
    ControllerButton down_button,
    ControllerButton left_button,
    ControllerButton right_button);
void bind_player_controller(ActionInputBindings& bindings, InputSlot slot, int controller_index);
void bind_player_move_axis(
    ActionInputBindings& bindings,
    InputSlot slot,
    ControllerAxis axis,
    bool invert_axis);
void bind_player_move_buttons(
    ActionInputBindings& bindings,
    InputSlot slot,
    ControllerButton up_button,
    ControllerButton down_button);
bool bind_keyboard_scancode_for_move_direction(
    ActionInputBindings& bindings,
    InputSlot slot,
    AxisDirection direction,
    int scancode);
bool bind_controller_button_for_move_direction(
    ActionInputBindings& bindings,
    InputSlot slot,
    AxisDirection direction,
    ControllerButton button);
void bind_controller_index_for_input_slot(ActionInputBindings& bindings, InputSlot slot, int controller_index);
int keyboard_scancode_for_move_direction(
    const ActionInputBindings& bindings,
    InputSlot slot,
    AxisDirection direction);
ControllerButton controller_button_for_move_direction(
    const ActionInputBindings& bindings,
    InputSlot slot,
    AxisDirection direction);
int controller_index_for_input_slot(const ActionInputBindings& bindings, InputSlot slot);
ControllerAxis controller_axis_for_input_slot(const ActionInputBindings& bindings, InputSlot slot);
bool controller_axis_inverted_for_input_slot(const ActionInputBindings& bindings, InputSlot slot);
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
