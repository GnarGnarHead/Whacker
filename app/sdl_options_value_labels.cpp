#include "sdl_options_value_labels.hpp"

#include <algorithm>

#include "options_menu_actions.hpp"
#include "sdl_input.hpp"
#include "sdl_options_binding_access.hpp"
#include "sdl_runtime_labels.hpp"

namespace whacker::app {

std::string sdl_options_value_label(
    const OptionsMenuState& menu_state,
    const int row,
    const void* raw_context) {
    const auto* context = static_cast<const SdlOptionsValueLabelContext*>(raw_context);
    if (context == nullptr) {
        return {};
    }
    if (options_row_is_binding(menu_state.section, row)) {
        const ActionInputBindings fallback = default_action_input_bindings();
        const ActionInputBindings& bindings =
            context->bindings != nullptr ? *context->bindings : fallback;
        InputSlot axis_slot = InputSlot::P1;
        if (sdl_options_axis_row(menu_state.section, row, axis_slot)) {
            return
                "C" + std::to_string(controller_index_for_options_row(bindings, menu_state.section, row)) + " " +
                controller_axis_label(controller_axis_for_options_row(bindings, menu_state.section, row));
        }
        return
            std::string(sdl_keyboard_scancode_label(
                keyboard_scancode_for_options_row(bindings, menu_state.section, row))) +
            " / " +
            "C" + std::to_string(controller_index_for_options_row(bindings, menu_state.section, row)) + " " +
            controller_button_label(controller_button_for_options_row(bindings, menu_state.section, row));
    }
    if (options_row_is_axis_invert(menu_state.section, row)) {
        const ActionInputBindings fallback = default_action_input_bindings();
        const ActionInputBindings& bindings =
            context->bindings != nullptr ? *context->bindings : fallback;
        return controller_axis_inverted_for_options_row(bindings, menu_state.section, row) ? "ON" : "OFF";
    }
    if (options_row_is_volume(menu_state.section, row)) {
        const int value =
            context->audio_settings != nullptr
                ? std::clamp(options_audio_value(*context->audio_settings, row), 0, 100)
                : 0;
        return std::to_string(value) + "%";
    }
    if (options_row_is_mute(menu_state.section, row)) {
        const bool enabled =
            context->audio_settings != nullptr && options_audio_toggle_value(*context->audio_settings, row);
        return enabled ? "ON" : "OFF";
    }
    return {};
}

}  // namespace whacker::app
