#include "sdl_input.hpp"

#ifndef WHACKER_PLATFORM_SDL2
#error "sdl_input.cpp must be compiled with WHACKER_PLATFORM_SDL2"
#endif

#if __has_include(<SDL2/SDL.h>)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

namespace whacker::app {

namespace {

bool key_down(const Uint8* keys, const SDL_Scancode scancode) {
    return keys != nullptr && keys[scancode] != 0;
}

KeyboardPhysicalState sample_keyboard_physical_state() {
    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    KeyboardPhysicalState state {};
    state.key_up = key_down(keys, SDL_SCANCODE_UP);
    state.key_down = key_down(keys, SDL_SCANCODE_DOWN);
    state.key_left = key_down(keys, SDL_SCANCODE_LEFT);
    state.key_right = key_down(keys, SDL_SCANCODE_RIGHT);
    state.key_w = key_down(keys, SDL_SCANCODE_W);
    state.key_s = key_down(keys, SDL_SCANCODE_S);
    state.key_enter = key_down(keys, SDL_SCANCODE_RETURN);
    state.key_kp_enter = key_down(keys, SDL_SCANCODE_KP_ENTER);
    state.key_space = key_down(keys, SDL_SCANCODE_SPACE);
    state.key_escape = key_down(keys, SDL_SCANCODE_ESCAPE);
    return state;
}

}  // namespace

void SdlKeyboardInput::sample() {
    previous_ = current_;
    current_ = sample_keyboard_physical_state();
    frame_ = derive_keyboard_action_frame(previous_, current_);
}

const ActionInputFrame& SdlKeyboardInput::frame() const {
    return frame_;
}

}  // namespace whacker::app
