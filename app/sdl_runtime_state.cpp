#include "sdl_runtime_state.hpp"

#include "input_binding_codec.hpp"
#include "menu_settings.hpp"
#include "sdl_runtime_audio.hpp"

namespace whacker::app {

void sync_controls_from_action_bindings(SdlRuntimeState& runtime) {
    sync_controls_from_action_bindings(runtime.controls, runtime.input.bindings());
}

void persist_runtime_menu_settings(const SdlRuntimeState& runtime) {
    save_menu_settings(
        runtime.options,
        runtime.controls,
        runtime.input.bindings(),
        runtime.audio_settings);
}

void initialize_sdl_runtime_state(SdlRuntimeState& runtime) {
    reset_to_root(runtime.navigation, Screen::MainMenu);
    runtime.app_state = navigation_app_state(runtime.navigation);
    load_menu_settings(
        runtime.options,
        runtime.controls,
        runtime.input.bindings(),
        runtime.audio_settings);
    sync_controls_from_action_bindings(runtime);
    initialize_sdl_runtime_audio(runtime);
}

void shutdown_sdl_runtime_state(SdlRuntimeState& runtime) {
    shutdown_sdl_runtime_audio(runtime);
}

}  // namespace whacker::app
