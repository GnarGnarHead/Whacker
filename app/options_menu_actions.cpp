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
        row == OptionsMenuRowP2Down ||
        row == OptionsMenuRowP1Axis ||
        row == OptionsMenuRowP2Axis;
}

bool options_row_is_axis_invert(const int row) {
    return row == OptionsMenuRowP1AxisInvert || row == OptionsMenuRowP2AxisInvert;
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
    const MenuIntent& intent) {
    if (options_menu_state.waiting_for_key) {
        if (intent.back || intent.confirm) {
            options_menu_state.waiting_for_key = false;
            return OptionsMenuActionResult::None;
        }
        if (intent.left || intent.right) {
            return OptionsMenuActionResult::BindingChanged;
        }
        return OptionsMenuActionResult::None;
    }

    if (intent.up) {
        options_menu_state.selected_row = (options_menu_state.selected_row + OptionsMenuRowCount - 1) % OptionsMenuRowCount;
    }
    if (intent.down) {
        options_menu_state.selected_row = (options_menu_state.selected_row + 1) % OptionsMenuRowCount;
    }
    if (intent.back) {
        options_menu_state.waiting_for_key = false;
        return OptionsMenuActionResult::Back;
    }

    if (options_row_is_volume(options_menu_state.selected_row)) {
        int delta = 0;
        if (intent.left) {
            delta -= 5;
        }
        if (intent.right) {
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
    } else if (options_row_is_axis_invert(options_menu_state.selected_row) && (intent.left || intent.right)) {
        return OptionsMenuActionResult::BindingChanged;
    } else if (options_row_is_mute(options_menu_state.selected_row) && (intent.left || intent.right)) {
        audio_settings.mute = !audio_settings.mute;
        return OptionsMenuActionResult::AudioChanged;
    }

    if (!intent.confirm) {
        return OptionsMenuActionResult::None;
    }

    if (options_menu_state.selected_row == OptionsMenuRowBack) {
        return OptionsMenuActionResult::Back;
    }
    if (options_row_is_binding(options_menu_state.selected_row)) {
        options_menu_state.waiting_for_key = true;
        return OptionsMenuActionResult::BindingCaptureStarted;
    }
    if (options_row_is_axis_invert(options_menu_state.selected_row)) {
        return OptionsMenuActionResult::BindingChanged;
    }
    if (options_row_is_mute(options_menu_state.selected_row)) {
        audio_settings.mute = !audio_settings.mute;
        return OptionsMenuActionResult::AudioChanged;
    }
    return OptionsMenuActionResult::None;
}

}  // namespace whacker::app
