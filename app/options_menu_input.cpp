#include "options_menu_input.hpp"

#ifdef WHACKER_HAS_GLFW

#include <algorithm>

#include <GLFW/glfw3.h>

namespace {

int* binding_slot(whacker::app::ControlBindings& bindings, const int row) {
    using namespace whacker::app;
    switch (row) {
        case OptionsMenuRowP1Up:
            return &bindings.p1_up;
        case OptionsMenuRowP1Down:
            return &bindings.p1_down;
        case OptionsMenuRowP2Up:
            return &bindings.p2_up;
        case OptionsMenuRowP2Down:
            return &bindings.p2_down;
        default:
            return nullptr;
    }
}

int* audio_volume_slot(whacker::app::AudioSettings& settings, const int row) {
    using namespace whacker::app;
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

namespace whacker::app {

int binding_value(const ControlBindings& bindings, const int row) {
    switch (row) {
        case OptionsMenuRowP1Up:
            return bindings.p1_up;
        case OptionsMenuRowP1Down:
            return bindings.p1_down;
        case OptionsMenuRowP2Up:
            return bindings.p2_up;
        case OptionsMenuRowP2Down:
            return bindings.p2_down;
        default:
            return GLFW_KEY_UNKNOWN;
    }
}

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
    if (row == OptionsMenuRowMute) {
        return settings.mute;
    }
    return false;
}

void handle_options_menu_input(
    GLFWwindow* window,
    KeyEdgeState& edge_state,
    OptionsMenuState& options_menu_state,
    ControlBindings& controls,
    AudioSettings& audio_settings,
    AppState& app_state,
    bool& changed_bindings,
    bool& changed_audio_settings) {
    changed_bindings = false;
    changed_audio_settings = false;
    if (options_menu_state.waiting_for_key) {
        const int key = consume_last_pressed_key();
        if (key == GLFW_KEY_ESCAPE) {
            options_menu_state.waiting_for_key = false;
            return;
        }
        if (is_bindable_key(key) && options_row_is_binding(options_menu_state.selected_row)) {
            int* slot = binding_slot(controls, options_menu_state.selected_row);
            if (slot != nullptr) {
                *slot = key;
                changed_bindings = true;
            }
            options_menu_state.waiting_for_key = false;
        }
        return;
    }

    if (consume_menu_up_press(window, edge_state, controls)) {
        options_menu_state.selected_row = (options_menu_state.selected_row + OptionsMenuRowCount - 1) % OptionsMenuRowCount;
    }
    if (consume_menu_down_press(window, edge_state, controls)) {
        options_menu_state.selected_row = (options_menu_state.selected_row + 1) % OptionsMenuRowCount;
    }

    if (options_row_is_volume(options_menu_state.selected_row)) {
        int delta = 0;
        if (consume_key_press(window, GLFW_KEY_LEFT, edge_state.left)) {
            delta -= 5;
        }
        if (consume_key_press(window, GLFW_KEY_RIGHT, edge_state.right)) {
            delta += 5;
        }
        if (delta != 0) {
            int* slot = audio_volume_slot(audio_settings, options_menu_state.selected_row);
            if (slot != nullptr) {
                const int before = *slot;
                *slot = std::clamp(*slot + delta, 0, 100);
                changed_audio_settings = changed_audio_settings || (*slot != before);
            }
        }
    } else if (options_row_is_mute(options_menu_state.selected_row)) {
        const bool left = consume_key_press(window, GLFW_KEY_LEFT, edge_state.left);
        const bool right = consume_key_press(window, GLFW_KEY_RIGHT, edge_state.right);
        if (left || right) {
            audio_settings.mute = !audio_settings.mute;
            changed_audio_settings = true;
        }
    }

    if (!consume_confirm_press(window, edge_state)) {
        return;
    }

    if (options_menu_state.selected_row == OptionsMenuRowBack) {
        app_state = AppState::MainMenu;
        return;
    }

    if (options_row_is_binding(options_menu_state.selected_row)) {
        options_menu_state.waiting_for_key = true;
        clear_last_pressed_key();
        return;
    }

    if (options_row_is_mute(options_menu_state.selected_row)) {
        audio_settings.mute = !audio_settings.mute;
        changed_audio_settings = true;
    }
}

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
