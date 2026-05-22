#include "options_menu_actions.hpp"

#include <algorithm>

namespace whacker::app {

namespace {

int* audio_volume_slot(AudioSettings& settings, const int row) {
    switch (row) {
        case OptionsMenuRowMasterVolume:
            return &settings.master_volume;
        case OptionsMenuRowMusicVolume:
            return &settings.music_volume;
        case OptionsMenuRowSfxVolume:
            return &settings.sfx_volume;
        default:
            return nullptr;
    }
}

}  // namespace

bool options_row_is_binding(const int row) {
    return
        row == OptionsMenuRowP1Up ||
        row == OptionsMenuRowP1Down ||
        row == OptionsMenuRowP2Up ||
        row == OptionsMenuRowP2Down;
}

bool options_row_is_volume(const int row) {
    return
        row == OptionsMenuRowMasterVolume ||
        row == OptionsMenuRowMusicVolume ||
        row == OptionsMenuRowSfxVolume;
}

bool options_row_is_mute(const int row) {
    return row == OptionsMenuRowMute;
}

int audio_value(const AudioSettings& settings, const int row) {
    switch (row) {
        case OptionsMenuRowMasterVolume:
            return settings.master_volume;
        case OptionsMenuRowMusicVolume:
            return settings.music_volume;
        case OptionsMenuRowSfxVolume:
            return settings.sfx_volume;
        default:
            return 0;
    }
}

bool audio_toggle_value(const AudioSettings& settings, const int row) {
    return row == OptionsMenuRowMute && settings.mute;
}

OptionsMenuActionResult apply_options_menu_action(
    OptionsMenuState& options_menu_state,
    AudioSettings& audio_settings,
    const bool move_up,
    const bool move_down,
    const bool move_left,
    const bool move_right,
    const bool confirm,
    const bool back) {
    if (options_menu_state.waiting_for_key) {
        if (back || confirm) {
            options_menu_state.waiting_for_key = false;
            return OptionsMenuActionResult::None;
        }
        if (move_left || move_right) {
            return OptionsMenuActionResult::BindingChanged;
        }
        return OptionsMenuActionResult::None;
    }

    if (move_up) {
        options_menu_state.selected_row = (options_menu_state.selected_row + OptionsMenuRowCount - 1) % OptionsMenuRowCount;
    }
    if (move_down) {
        options_menu_state.selected_row = (options_menu_state.selected_row + 1) % OptionsMenuRowCount;
    }
    if (back) {
        options_menu_state.waiting_for_key = false;
        return OptionsMenuActionResult::Back;
    }

    if (options_row_is_volume(options_menu_state.selected_row)) {
        int delta = 0;
        if (move_left) {
            delta -= 5;
        }
        if (move_right) {
            delta += 5;
        }
        if (delta != 0) {
            int* slot = audio_volume_slot(audio_settings, options_menu_state.selected_row);
            if (slot != nullptr) {
                const int before = *slot;
                *slot = std::clamp(*slot + delta, 0, 100);
                return *slot != before ? OptionsMenuActionResult::AudioChanged : OptionsMenuActionResult::None;
            }
        }
    } else if (options_row_is_mute(options_menu_state.selected_row) && (move_left || move_right)) {
        audio_settings.mute = !audio_settings.mute;
        return OptionsMenuActionResult::AudioChanged;
    }

    if (!confirm) {
        return OptionsMenuActionResult::None;
    }

    if (options_menu_state.selected_row == OptionsMenuRowBack) {
        return OptionsMenuActionResult::Back;
    }
    if (options_row_is_binding(options_menu_state.selected_row)) {
        options_menu_state.waiting_for_key = true;
        return OptionsMenuActionResult::BindingCaptureStarted;
    }
    if (options_row_is_mute(options_menu_state.selected_row)) {
        audio_settings.mute = !audio_settings.mute;
        return OptionsMenuActionResult::AudioChanged;
    }
    return OptionsMenuActionResult::None;
}

OptionsMenuActionResult apply_options_menu_action_frame(
    OptionsMenuState& options_menu_state,
    AudioSettings& audio_settings,
    const ActionInputFrame& input) {
    return apply_options_menu_action(
        options_menu_state,
        audio_settings,
        input_pressed(input, InputAction::MenuUp),
        input_pressed(input, InputAction::MenuDown),
        input_pressed(input, InputAction::MenuLeft),
        input_pressed(input, InputAction::MenuRight),
        input_pressed(input, InputAction::Confirm),
        input_pressed(input, InputAction::Back));
}

}  // namespace whacker::app
