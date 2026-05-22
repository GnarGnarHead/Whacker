#include "sdl_input.hpp"

#ifndef WHACKER_PLATFORM_SDL2
#error "sdl_input.cpp must be compiled with WHACKER_PLATFORM_SDL2"
#endif

#if __has_include(<SDL2/SDL.h>)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

namespace whacker::app {

namespace {

KeyboardPhysicalState sample_keyboard_physical_state() {
    int key_count = 0;
    const Uint8* keys = SDL_GetKeyboardState(&key_count);
    KeyboardPhysicalState state {};
    const int sampled_count = std::min(key_count, kKeyboardScancodeCount);
    for (int i = 0; i < sampled_count; ++i) {
        state.scancodes[static_cast<std::size_t>(i)] = keys != nullptr && keys[i] != 0;
    }
    return state;
}

float normalize_axis(const Sint16 value) {
    if (value < 0) {
        return static_cast<float>(value) / 32768.0f;
    }
    return static_cast<float>(value) / 32767.0f;
}

SDL_GameControllerAxis sdl_axis(const ControllerAxis axis) {
    switch (axis) {
        case ControllerAxis::LeftX:
            return SDL_CONTROLLER_AXIS_LEFTX;
        case ControllerAxis::LeftY:
            return SDL_CONTROLLER_AXIS_LEFTY;
        case ControllerAxis::RightX:
            return SDL_CONTROLLER_AXIS_RIGHTX;
        case ControllerAxis::RightY:
            return SDL_CONTROLLER_AXIS_RIGHTY;
        case ControllerAxis::Count:
            return SDL_CONTROLLER_AXIS_INVALID;
    }
    return SDL_CONTROLLER_AXIS_INVALID;
}

SDL_GameControllerButton sdl_button(const ControllerButton button) {
    switch (button) {
        case ControllerButton::A:
            return SDL_CONTROLLER_BUTTON_A;
        case ControllerButton::B:
            return SDL_CONTROLLER_BUTTON_B;
        case ControllerButton::X:
            return SDL_CONTROLLER_BUTTON_X;
        case ControllerButton::Y:
            return SDL_CONTROLLER_BUTTON_Y;
        case ControllerButton::Back:
            return SDL_CONTROLLER_BUTTON_BACK;
        case ControllerButton::Guide:
            return SDL_CONTROLLER_BUTTON_GUIDE;
        case ControllerButton::Start:
            return SDL_CONTROLLER_BUTTON_START;
        case ControllerButton::LeftStick:
            return SDL_CONTROLLER_BUTTON_LEFTSTICK;
        case ControllerButton::RightStick:
            return SDL_CONTROLLER_BUTTON_RIGHTSTICK;
        case ControllerButton::LeftShoulder:
            return SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
        case ControllerButton::RightShoulder:
            return SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
        case ControllerButton::DpadUp:
            return SDL_CONTROLLER_BUTTON_DPAD_UP;
        case ControllerButton::DpadDown:
            return SDL_CONTROLLER_BUTTON_DPAD_DOWN;
        case ControllerButton::DpadLeft:
            return SDL_CONTROLLER_BUTTON_DPAD_LEFT;
        case ControllerButton::DpadRight:
            return SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
        case ControllerButton::Count:
        case ControllerButton::Unbound:
            return SDL_CONTROLLER_BUTTON_INVALID;
    }
    return SDL_CONTROLLER_BUTTON_INVALID;
}

SDL_GameController* controller_handle(void* handle) {
    return static_cast<SDL_GameController*>(handle);
}

int free_controller_slot(const std::array<void*, kMaxInputControllers>& controllers) {
    for (int index = 0; index < kMaxInputControllers; ++index) {
        if (controllers[static_cast<std::size_t>(index)] == nullptr) {
            return index;
        }
    }
    return -1;
}

bool controller_instance_open(
    const std::array<int, kMaxInputControllers>& instance_ids,
    const SDL_JoystickID instance_id) {
    if (instance_id < 0) {
        return false;
    }
    for (const int open_instance_id : instance_ids) {
        if (open_instance_id == static_cast<int>(instance_id)) {
            return true;
        }
    }
    return false;
}

}  // namespace

SdlInput::SdlInput()
    : SdlInput(default_action_input_bindings()) {}

SdlInput::SdlInput(ActionInputBindings bindings)
    : bindings_(bindings) {
    controller_instance_ids_.fill(-1);
}

SdlInput::~SdlInput() {
    close_controllers();
}

void SdlInput::sample() {
    refresh_controllers();
    SDL_GameControllerUpdate();
    previous_ = current_;
    current_ = sample_physical_state();
    frame_ = derive_action_input_frame(previous_, current_, bindings_);
}

const ActionInputFrame& SdlInput::frame() const {
    return frame_;
}

ActionInputBindings& SdlInput::bindings() {
    return bindings_;
}

const ActionInputBindings& SdlInput::bindings() const {
    return bindings_;
}

int SdlInput::controller_index_for_instance_id(const int instance_id) const {
    if (instance_id < 0) {
        return -1;
    }
    for (int slot = 0; slot < kMaxInputControllers; ++slot) {
        if (controller_instance_ids_[static_cast<std::size_t>(slot)] == instance_id) {
            return slot;
        }
    }
    return -1;
}

const char* sdl_keyboard_scancode_label(const int scancode) {
    if (scancode < 0 || scancode >= kKeyboardScancodeCount) {
        return "UNBOUND";
    }
    const char* name = SDL_GetScancodeName(static_cast<SDL_Scancode>(scancode));
    if (name == nullptr || name[0] == '\0') {
        return "?";
    }
    return name;
}

bool sdl_keyboard_scancode_bindable(const int scancode) {
    if (!keyboard_scancode_bindable(scancode)) {
        return false;
    }
    const char* name = SDL_GetScancodeName(static_cast<SDL_Scancode>(scancode));
    return name != nullptr && name[0] != '\0';
}

void SdlInput::refresh_controllers() {
    for (int slot = 0; slot < kMaxInputControllers; ++slot) {
        void*& handle = controllers_[static_cast<std::size_t>(slot)];
        if (handle == nullptr) {
            continue;
        }
        SDL_GameController* controller = controller_handle(handle);
        if (SDL_GameControllerGetAttached(controller) == SDL_FALSE) {
            SDL_GameControllerClose(controller);
            handle = nullptr;
            controller_instance_ids_[static_cast<std::size_t>(slot)] = -1;
        }
    }

    const int joystick_count = SDL_NumJoysticks();
    for (int joystick_index = 0; joystick_index < joystick_count; ++joystick_index) {
        if (SDL_IsGameController(joystick_index) == SDL_FALSE) {
            continue;
        }
        const SDL_JoystickID instance_id = SDL_JoystickGetDeviceInstanceID(joystick_index);
        if (controller_instance_open(controller_instance_ids_, instance_id)) {
            continue;
        }
        const int slot = free_controller_slot(controllers_);
        if (slot < 0) {
            return;
        }
        SDL_GameController* controller = SDL_GameControllerOpen(joystick_index);
        if (controller == nullptr) {
            continue;
        }
        controllers_[static_cast<std::size_t>(slot)] = controller;
        SDL_Joystick* joystick = SDL_GameControllerGetJoystick(controller);
        controller_instance_ids_[static_cast<std::size_t>(slot)] = static_cast<int>(
            joystick != nullptr ? SDL_JoystickInstanceID(joystick) : instance_id);
    }
}

void SdlInput::close_controllers() {
    for (int slot = 0; slot < kMaxInputControllers; ++slot) {
        void*& handle = controllers_[static_cast<std::size_t>(slot)];
        if (handle != nullptr) {
            SDL_GameControllerClose(controller_handle(handle));
            handle = nullptr;
        }
        controller_instance_ids_[static_cast<std::size_t>(slot)] = -1;
    }
}

InputPhysicalState SdlInput::sample_physical_state() const {
    InputPhysicalState state {};
    state.keyboard = sample_keyboard_physical_state();
    for (int slot = 0; slot < kMaxInputControllers; ++slot) {
        SDL_GameController* controller = controller_handle(controllers_[static_cast<std::size_t>(slot)]);
        if (controller == nullptr) {
            continue;
        }
        ControllerPhysicalState& controller_state = state.controllers[static_cast<std::size_t>(slot)];
        controller_state.connected = SDL_GameControllerGetAttached(controller) == SDL_TRUE;
        if (!controller_state.connected) {
            continue;
        }
        for (int axis_index = 0; axis_index < kControllerAxisCount; ++axis_index) {
            const ControllerAxis axis = static_cast<ControllerAxis>(axis_index);
            const SDL_GameControllerAxis mapped_axis = sdl_axis(axis);
            if (mapped_axis != SDL_CONTROLLER_AXIS_INVALID) {
                controller_state.axes[static_cast<std::size_t>(axis_index)] =
                    normalize_axis(SDL_GameControllerGetAxis(controller, mapped_axis));
            }
        }
        for (int button_index = 0; button_index < kControllerButtonCount; ++button_index) {
            const ControllerButton button = static_cast<ControllerButton>(button_index);
            const SDL_GameControllerButton mapped_button = sdl_button(button);
            if (mapped_button != SDL_CONTROLLER_BUTTON_INVALID) {
                controller_state.buttons[static_cast<std::size_t>(button_index)] =
                    SDL_GameControllerGetButton(controller, mapped_button) != 0;
            }
        }
    }
    return state;
}

}  // namespace whacker::app
