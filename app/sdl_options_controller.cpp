#include "sdl_options_controller.hpp"

#include "options_menu_actions.hpp"
#include "sdl_options_capture.hpp"

namespace whacker::app {

SdlOptionsUpdateEffects update_sdl_options_menu(
    SdlRuntimeState& runtime,
    const MenuIntent& intent,
    const SdlEventFrame& events) {
    SdlOptionsUpdateEffects effects {};
    const int row_before = runtime.options_menu.selected_row;
    const int selected_row = runtime.options_menu.selected_row;

    if (runtime.options_menu.waiting_for_key) {
        const SdlOptionsCaptureResult capture = apply_sdl_options_capture(
            runtime.options_menu,
            runtime.input.bindings(),
            runtime.controls,
            runtime.input,
            events);
        if (capture.finished) {
            effects.binding_changed = capture.binding_changed;
            effects.persist_requested = capture.binding_changed;
            effects.play_confirm_sound = capture.binding_changed;
            return effects;
        }
    }

    const OptionsMenuActionResult result = apply_options_menu_action(
        runtime.options_menu,
        runtime.audio_settings,
        intent);
    if (runtime.options_menu.selected_row != row_before) {
        effects.play_move_sound = true;
    }
    if (result == OptionsMenuActionResult::BindingChanged) {
        const int direction = intent.right ? 1 : (intent.left ? -1 : 0);
        if (options_row_is_axis_invert(selected_row)) {
            effects.binding_changed = toggle_sdl_options_controller_axis_invert(
                runtime.input.bindings(),
                selected_row);
        } else {
            effects.binding_changed =
                cycle_sdl_options_controller_axis(
                    runtime.input.bindings(),
                    selected_row,
                    direction) ||
                cycle_sdl_options_controller_button(
                    runtime.input.bindings(),
                    selected_row,
                    direction);
        }
        effects.persist_requested = effects.binding_changed;
        effects.play_move_sound = true;
    } else if (result == OptionsMenuActionResult::AudioChanged) {
        runtime.audio_settings = clamp_audio_settings(runtime.audio_settings);
        effects.audio_changed = true;
        effects.persist_requested = true;
        effects.play_move_sound = true;
    } else if (result == OptionsMenuActionResult::BindingCaptureStarted) {
        effects.play_confirm_sound = true;
    } else if (result == OptionsMenuActionResult::Back) {
        effects.back_requested = true;
        effects.play_confirm_sound = true;
    }
    return effects;
}

}  // namespace whacker::app
