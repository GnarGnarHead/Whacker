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

enum class OptionsMenuSection : std::uint8_t {
    Root,
    Controls,
    Audio
};

enum OptionsRootMenuRow : std::uint8_t {
    OptionsRootRowControls = 0,
    OptionsRootRowAudio = 1,
    OptionsRootRowBack = 2,
    OptionsRootRowCount = 3
};

enum OptionsControlsMenuRow : std::uint8_t {
    OptionsControlsRowPreset = 0,
    OptionsControlsRowP1Up = 1,
    OptionsControlsRowP1Down = 2,
    OptionsControlsRowP1Axis = 3,
    OptionsControlsRowP1AxisInvert = 4,
    OptionsControlsRowP2Up = 5,
    OptionsControlsRowP2Down = 6,
    OptionsControlsRowP2Axis = 7,
    OptionsControlsRowP2AxisInvert = 8,
    OptionsControlsRowBack = 9,
    OptionsControlsRowCount = 10
};

enum OptionsAudioMenuRow : std::uint8_t {
    OptionsAudioRowMasterVolume = 0,
    OptionsAudioRowMusicVolume = 1,
    OptionsAudioRowSfxVolume = 2,
    OptionsAudioRowMute = 3,
    OptionsAudioRowBack = 4,
    OptionsAudioRowCount = 5
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
    OptionsMenuSection section = OptionsMenuSection::Root;
    int selected_row = 0;
    bool waiting_for_input = false;
};

struct PauseMenuState {
    int selected_row = PauseMenuRowResume;
    bool confirm_forfeit = false;
    int confirm_selected = 0;
};

}  // namespace whacker::app
