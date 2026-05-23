#include "options_menu_actions.hpp"

#include <algorithm>

namespace whacker::app {

namespace {

int* audio_volume_slot(AudioSettings& settings, const int row) {
    switch (row) {
        case OptionsAudioRowMasterVolume:
            return &settings.master_volume;
        case OptionsAudioRowMusicVolume:
            return &settings.music_volume;
        case OptionsAudioRowSfxVolume:
            return &settings.sfx_volume;
        default:
            return nullptr;
    }
}

void enter_options_section(OptionsMenuState& state, const OptionsMenuSection section) {
    state.section = section;
    state.selected_row = 0;
    state.waiting_for_input = false;
}

OptionsMenuActionResult apply_root_action(OptionsMenuState& state, const MenuIntent& intent) {
    if (!intent.confirm) {
        return OptionsMenuActionResult::None;
    }
    switch (state.selected_row) {
        case OptionsRootRowControls:
            enter_options_section(state, OptionsMenuSection::Controls);
            return OptionsMenuActionResult::SectionChanged;
        case OptionsRootRowAudio:
            enter_options_section(state, OptionsMenuSection::Audio);
            return OptionsMenuActionResult::SectionChanged;
        case OptionsRootRowBack:
            return OptionsMenuActionResult::Back;
        default:
            return OptionsMenuActionResult::None;
    }
}

OptionsMenuActionResult apply_controls_action(OptionsMenuState& state, const MenuIntent& intent) {
    if (intent.left || intent.right) {
        if (options_row_is_control_preset(state.section, state.selected_row)) {
            return OptionsMenuActionResult::ControlPresetChanged;
        }
        if (options_row_is_binding(state.section, state.selected_row) ||
            options_row_is_axis_invert(state.section, state.selected_row)) {
            return OptionsMenuActionResult::BindingChanged;
        }
    }

    if (!intent.confirm) {
        return OptionsMenuActionResult::None;
    }
    if (state.selected_row == OptionsControlsRowBack) {
        enter_options_section(state, OptionsMenuSection::Root);
        return OptionsMenuActionResult::SectionChanged;
    }
    if (options_row_is_control_preset(state.section, state.selected_row)) {
        return OptionsMenuActionResult::ControlPresetChanged;
    }
    if (options_row_is_binding(state.section, state.selected_row)) {
        state.waiting_for_input = true;
        return OptionsMenuActionResult::BindingCaptureStarted;
    }
    if (options_row_is_axis_invert(state.section, state.selected_row)) {
        return OptionsMenuActionResult::BindingChanged;
    }
    return OptionsMenuActionResult::None;
}

OptionsMenuActionResult apply_audio_action(
    OptionsMenuState& state,
    AudioSettings& audio_settings,
    const MenuIntent& intent) {
    if (options_row_is_volume(state.section, state.selected_row)) {
        int delta = 0;
        if (intent.left) {
            delta -= 5;
        }
        if (intent.right) {
            delta += 5;
        }
        if (delta != 0) {
            int* slot = audio_volume_slot(audio_settings, state.selected_row);
            if (slot != nullptr) {
                const int before = *slot;
                *slot = std::clamp(*slot + delta, 0, 100);
                return *slot != before ? OptionsMenuActionResult::AudioChanged : OptionsMenuActionResult::None;
            }
        }
    } else if (options_row_is_mute(state.section, state.selected_row) && (intent.left || intent.right)) {
        audio_settings.mute = !audio_settings.mute;
        return OptionsMenuActionResult::AudioChanged;
    }

    if (!intent.confirm) {
        return OptionsMenuActionResult::None;
    }
    if (state.selected_row == OptionsAudioRowBack) {
        enter_options_section(state, OptionsMenuSection::Root);
        return OptionsMenuActionResult::SectionChanged;
    }
    if (options_row_is_mute(state.section, state.selected_row)) {
        audio_settings.mute = !audio_settings.mute;
        return OptionsMenuActionResult::AudioChanged;
    }
    return OptionsMenuActionResult::None;
}

}  // namespace

int options_menu_row_count(const OptionsMenuSection section) {
    switch (section) {
        case OptionsMenuSection::Root:
            return OptionsRootRowCount;
        case OptionsMenuSection::Controls:
            return OptionsControlsRowCount;
        case OptionsMenuSection::Audio:
            return OptionsAudioRowCount;
    }
    return OptionsRootRowCount;
}

bool options_row_is_back(const OptionsMenuSection section, const int row) {
    switch (section) {
        case OptionsMenuSection::Root:
            return row == OptionsRootRowBack;
        case OptionsMenuSection::Controls:
            return row == OptionsControlsRowBack;
        case OptionsMenuSection::Audio:
            return row == OptionsAudioRowBack;
    }
    return false;
}

bool options_row_is_control_preset(const OptionsMenuSection section, const int row) {
    return section == OptionsMenuSection::Controls && row == OptionsControlsRowPreset;
}

bool options_row_is_binding(const OptionsMenuSection section, const int row) {
    if (section != OptionsMenuSection::Controls) {
        return false;
    }
    return
        row == OptionsControlsRowP1Up ||
        row == OptionsControlsRowP1Down ||
        row == OptionsControlsRowP2Up ||
        row == OptionsControlsRowP2Down ||
        row == OptionsControlsRowP1Axis ||
        row == OptionsControlsRowP2Axis;
}

bool options_row_is_axis_invert(const OptionsMenuSection section, const int row) {
    return
        section == OptionsMenuSection::Controls &&
        (row == OptionsControlsRowP1AxisInvert || row == OptionsControlsRowP2AxisInvert);
}

bool options_row_is_volume(const OptionsMenuSection section, const int row) {
    if (section != OptionsMenuSection::Audio) {
        return false;
    }
    return
        row == OptionsAudioRowMasterVolume ||
        row == OptionsAudioRowMusicVolume ||
        row == OptionsAudioRowSfxVolume;
}

bool options_row_is_mute(const OptionsMenuSection section, const int row) {
    return section == OptionsMenuSection::Audio && row == OptionsAudioRowMute;
}

int options_audio_value(const AudioSettings& settings, const int row) {
    switch (row) {
        case OptionsAudioRowMasterVolume:
            return settings.master_volume;
        case OptionsAudioRowMusicVolume:
            return settings.music_volume;
        case OptionsAudioRowSfxVolume:
            return settings.sfx_volume;
        default:
            return 0;
    }
}

bool options_audio_toggle_value(const AudioSettings& settings, const int row) {
    return row == OptionsAudioRowMute && settings.mute;
}

OptionsMenuActionResult apply_options_menu_action(
    OptionsMenuState& options_menu_state,
    AudioSettings& audio_settings,
    const MenuIntent& intent) {
    if (options_menu_state.waiting_for_input) {
        if (intent.back || intent.confirm) {
            options_menu_state.waiting_for_input = false;
            return OptionsMenuActionResult::None;
        }
        if (intent.left || intent.right) {
            return OptionsMenuActionResult::BindingChanged;
        }
        return OptionsMenuActionResult::None;
    }

    const int row_count = options_menu_row_count(options_menu_state.section);
    if (intent.up) {
        options_menu_state.selected_row = (options_menu_state.selected_row + row_count - 1) % row_count;
    }
    if (intent.down) {
        options_menu_state.selected_row = (options_menu_state.selected_row + 1) % row_count;
    }
    if (intent.back) {
        if (options_menu_state.section == OptionsMenuSection::Root) {
            return OptionsMenuActionResult::Back;
        }
        enter_options_section(options_menu_state, OptionsMenuSection::Root);
        return OptionsMenuActionResult::SectionChanged;
    }

    switch (options_menu_state.section) {
        case OptionsMenuSection::Root:
            return apply_root_action(options_menu_state, intent);
        case OptionsMenuSection::Controls:
            return apply_controls_action(options_menu_state, intent);
        case OptionsMenuSection::Audio:
            return apply_audio_action(options_menu_state, audio_settings, intent);
    }
    return OptionsMenuActionResult::None;
}

}  // namespace whacker::app
