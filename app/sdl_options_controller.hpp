#pragma once

#include "menu_intent.hpp"
#include "platform_sdl.hpp"
#include "sdl_runtime_state.hpp"

namespace whacker::app {

struct SdlOptionsUpdateEffects {
    bool binding_changed = false;
    bool audio_changed = false;
    bool back_requested = false;
    bool play_move_sound = false;
    bool play_confirm_sound = false;
    bool persist_requested = false;
};

SdlOptionsUpdateEffects update_sdl_options_menu(
    SdlRuntimeState& runtime,
    const MenuIntent& intent,
    const SdlEventFrame& events);

}  // namespace whacker::app
