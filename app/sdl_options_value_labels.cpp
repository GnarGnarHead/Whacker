#include "sdl_options_value_labels.hpp"

#include <algorithm>

#include "options_menu_actions.hpp"
#include "sdl_input.hpp"
#include "sdl_options_binding_access.hpp"
#include "sdl_runtime_labels.hpp"

namespace whacker::app {

std::string sdl_options_value_label(const int row, const void* raw_context) {
    const auto* context = static_cast<const SdlOptionsValueLabelContext*>(raw_context);
    if (context == nullptr) {
        return {};
    }
    if (options_row_is_binding(row)) {
        const ActionInputBindings fallback = default_action_input_bindings();
        const ActionInputBindings& bindings =
            context->bindings != nullptr ? *context->bindings : fallback;
        InputSlot axis_slot = InputSlot::P1;
        if (sdl_options_axis_row(row, axis_slot)) {
            return
                "C" + std::to_string(controller_index_for_options_row(bindings, row)) + " " +
                controller_axis_label(controller_axis_for_options_row(bindings, row));
        }
        return
            std::string(sdl_keyboard_scancode_label(keyboard_scancode_for_options_row(bindings, row))) +
            " / " +
            "C" + std::to_string(controller_index_for_options_row(bindings, row)) + " " +
            controller_button_label(controller_button_for_options_row(bindings, row));
    }
    if (options_row_is_axis_invert(row)) {
        const ActionInputBindings fallback = default_action_input_bindings();
        const ActionInputBindings& bindings =
            context->bindings != nullptr ? *context->bindings : fallback;
        return controller_axis_inverted_for_options_row(bindings, row) ? "ON" : "OFF";
    }
    if (options_row_is_volume(row)) {
        const int value =
            context->audio_settings != nullptr
                ? std::clamp(audio_value(*context->audio_settings, row), 0, 100)
                : 0;
        return std::to_string(value) + "%";
    }
    if (options_row_is_mute(row)) {
        const bool enabled =
            context->audio_settings != nullptr && audio_toggle_value(*context->audio_settings, row);
        return enabled ? "ON" : "OFF";
    }
    return {};
}

}  // namespace whacker::app
