#pragma once

#include <cstdint>
#include <vector>

#include "ui_state.hpp"

namespace whacker::app {

enum class Screen : std::uint8_t {
    MainMenu,
    QuickMatchSetup,
    OptionsMenu,
    PaddleTuning,
    StoryMenu,
    StoryHub,
    StoryScene,
    StoryIntro,
    Playing,
    Paused,
};

struct NavigationState {
    Screen current = Screen::MainMenu;
    std::vector<Screen> stack {};
};

struct ScreenRoute {
    bool changed = false;
    Screen screen = Screen::MainMenu;
};

constexpr Screen screen_for_app_state(const AppState state) {
    switch (state) {
        case AppState::MainMenu:
            return Screen::MainMenu;
        case AppState::QuickMatchSetup:
            return Screen::QuickMatchSetup;
        case AppState::OptionsMenu:
            return Screen::OptionsMenu;
        case AppState::PaddleTuning:
            return Screen::PaddleTuning;
        case AppState::StoryMenu:
            return Screen::StoryMenu;
        case AppState::StoryHub:
            return Screen::StoryHub;
        case AppState::StoryScene:
            return Screen::StoryScene;
        case AppState::StoryIntro:
            return Screen::StoryIntro;
        case AppState::Playing:
            return Screen::Playing;
        case AppState::Paused:
            return Screen::Paused;
        default:
            return Screen::MainMenu;
    }
}

constexpr AppState app_state_for_screen(const Screen screen) {
    switch (screen) {
        case Screen::MainMenu:
            return AppState::MainMenu;
        case Screen::QuickMatchSetup:
            return AppState::QuickMatchSetup;
        case Screen::OptionsMenu:
            return AppState::OptionsMenu;
        case Screen::PaddleTuning:
            return AppState::PaddleTuning;
        case Screen::StoryMenu:
            return AppState::StoryMenu;
        case Screen::StoryHub:
            return AppState::StoryHub;
        case Screen::StoryScene:
            return AppState::StoryScene;
        case Screen::StoryIntro:
            return AppState::StoryIntro;
        case Screen::Playing:
            return AppState::Playing;
        case Screen::Paused:
            return AppState::Paused;
        default:
            return AppState::MainMenu;
    }
}
constexpr ScreenRoute no_screen_route() { return ScreenRoute {}; }
constexpr ScreenRoute screen_route(const Screen screen) {
    return ScreenRoute {.changed = true, .screen = screen};
}

void push_screen(NavigationState& navigation, Screen screen);
bool pop_screen(NavigationState& navigation);
void replace_screen(NavigationState& navigation, Screen screen);
void reset_to_root(NavigationState& navigation, Screen screen);

bool has_previous_screen(const NavigationState& navigation);
Screen previous_screen_or(const NavigationState& navigation, Screen fallback);
AppState navigation_app_state(const NavigationState& navigation);

}  // namespace whacker::app
