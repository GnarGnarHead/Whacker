#pragma once

#include <cstdint>

namespace whacker::app {

enum class AppState : std::uint8_t {
    MainMenu,
    OptionsMenu,
    QuickMatchSetup,
    PaddleTuning,
    StoryMenu,
    StoryIntro,
    StoryScene,
    StoryHub,
    Playing,
    Paused
};

enum MainMenuRow : std::uint8_t {
    MainMenuRowStory = 0,
    MainMenuRowQuick = 1,
    MainMenuRowOptions = 2,
    MainMenuRowQuit = 3,
    MainMenuRowCount = 4
};

enum StoryMenuRow : std::uint8_t {
    StoryMenuRowContinue = 0,
    StoryMenuRowNewCareer = 1,
    StoryMenuRowBack = 2,
    StoryMenuRowCount = 3
};

enum StoryHubRow : std::uint8_t {
    StoryHubRowOfficialMatch = 0,
    StoryHubRowTrainingMatch = 1,
    StoryHubRowNextWeek = 2,
    StoryHubRowPaddleTuning = 3,
    StoryHubRowBack = 4,
    StoryHubRowCount = 5
};

enum MenuRow : std::uint8_t {
    MenuRowP1 = 0,
    MenuRowP2 = 1,
    MenuRowP1Tuning = 2,
    MenuRowP2Tuning = 3,
    MenuRowStart = 4,
    MenuRowCount = 5
};

enum OptionsMenuRow : std::uint8_t {
    OptionsMenuRowP1Up = 0,
    OptionsMenuRowP1Down = 1,
    OptionsMenuRowP2Up = 2,
    OptionsMenuRowP2Down = 3,
    OptionsMenuRowP1Axis = 4,
    OptionsMenuRowP1AxisInvert = 5,
    OptionsMenuRowP2Axis = 6,
    OptionsMenuRowP2AxisInvert = 7,
    OptionsMenuRowMasterVolume = 8,
    OptionsMenuRowMusicVolume = 9,
    OptionsMenuRowSfxVolume = 10,
    OptionsMenuRowMute = 11,
    OptionsMenuRowBack = 12,
    OptionsMenuRowCount = 13
};

enum PauseMenuRow : std::uint8_t {
    PauseMenuRowResume = 0,
    PauseMenuRowExitMatch = 1,
    PauseMenuRowQuitToMainMenu = 2,
    PauseMenuRowCount = 3
};

struct MenuState {
    int selected_row = 0;
};

struct MainMenuState {
    int selected_row = 0;
};

struct StoryMenuState {
    int selected_row = 0;
    bool confirm_overwrite = false;
    int confirm_selected = 0;
};

struct OptionsMenuState {
    int selected_row = 0;
    bool waiting_for_key = false;
};

struct PauseMenuState {
    int selected_row = PauseMenuRowResume;
    bool confirm_forfeit = false;
    int confirm_selected = 0;
};

}  // namespace whacker::app
