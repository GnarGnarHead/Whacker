#include "sdl_runtime_labels.hpp"

#include "sdl_input.hpp"
#include "ui_state.hpp"

namespace whacker::app {

const char* main_menu_row_name(const int row) {
    switch (row) {
        case MainMenuRowStory:
            return "STORY MODE";
        case MainMenuRowQuick:
            return "QUICK MATCH";
        case MainMenuRowOptions:
            return "OPTIONS";
        case MainMenuRowQuit:
            return "QUIT";
        default:
            return "?";
    }
}

const char* options_menu_row_name(const int row) {
    switch (row) {
        case OptionsMenuRowP1Up:
            return "P1 UP";
        case OptionsMenuRowP1Down:
            return "P1 DOWN";
        case OptionsMenuRowP2Up:
            return "P2 UP";
        case OptionsMenuRowP2Down:
            return "P2 DOWN";
        case OptionsMenuRowMasterVolume:
            return "MASTER VOLUME";
        case OptionsMenuRowMusicVolume:
            return "MUSIC VOLUME";
        case OptionsMenuRowSfxVolume:
            return "SFX VOLUME";
        case OptionsMenuRowMute:
            return "MUTE";
        case OptionsMenuRowBack:
            return "BACK";
        default:
            return "?";
    }
}

const char* story_menu_row_name(const int row) {
    switch (row) {
        case StoryMenuRowContinue:
            return "CONTINUE";
        case StoryMenuRowNewCareer:
            return "NEW CAREER";
        case StoryMenuRowBack:
            return "BACK";
        default:
            return "?";
    }
}

const char* story_hub_row_name(const int row) {
    switch (row) {
        case StoryHubRowOfficialMatch:
            return "NEXT MATCH";
        case StoryHubRowTrainingMatch:
            return "TRAINING MATCH";
        case StoryHubRowNextWeek:
            return "ADVANCE STORY";
        case StoryHubRowPaddleTuning:
            return "PADDLE TUNING";
        case StoryHubRowBack:
            return "BACK";
        default:
            return "?";
    }
}

const char* controller_button_label(const ControllerButton button) {
    switch (button) {
        case ControllerButton::A:
            return "A";
        case ControllerButton::B:
            return "B";
        case ControllerButton::X:
            return "X";
        case ControllerButton::Y:
            return "Y";
        case ControllerButton::Back:
            return "BACK";
        case ControllerButton::Guide:
            return "GUIDE";
        case ControllerButton::Start:
            return "START";
        case ControllerButton::LeftStick:
            return "L STICK";
        case ControllerButton::RightStick:
            return "R STICK";
        case ControllerButton::LeftShoulder:
            return "L SHOULDER";
        case ControllerButton::RightShoulder:
            return "R SHOULDER";
        case ControllerButton::DpadUp:
            return "DPAD UP";
        case ControllerButton::DpadDown:
            return "DPAD DOWN";
        case ControllerButton::DpadLeft:
            return "DPAD LEFT";
        case ControllerButton::DpadRight:
            return "DPAD RIGHT";
        case ControllerButton::Count:
        case ControllerButton::Unbound:
            return "UNBOUND";
    }
    return "UNBOUND";
}

const char* sdl_key_name(const int key) {
    return sdl_keyboard_scancode_label(key);
}

}  // namespace whacker::app
