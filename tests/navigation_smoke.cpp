#include "navigation.hpp"

#include <cstdlib>
#include <iostream>

namespace {

void require(const bool condition, const char* message) {
    if (!condition) {
        std::cerr << "navigation_smoke failed: " << message << "\n";
        std::exit(1);
    }
}

void test_app_state_mapping() {
    require(
        whacker::app::app_state_for_screen(whacker::app::screen_for_app_state(whacker::app::AppState::StoryHub)) ==
            whacker::app::AppState::StoryHub,
        "StoryHub maps round-trip");
    require(
        whacker::app::app_state_for_screen(whacker::app::screen_for_app_state(whacker::app::AppState::Paused)) ==
            whacker::app::AppState::Paused,
        "Paused maps round-trip");
}

void test_push_pop_replace_and_reset() {
    whacker::app::NavigationState navigation {};
    require(navigation.current == whacker::app::Screen::MainMenu, "default root is main menu");
    require(!whacker::app::pop_screen(navigation), "root back is a no-op");
    require(navigation.current == whacker::app::Screen::MainMenu, "root remains main menu after empty pop");

    whacker::app::push_screen(navigation, whacker::app::Screen::OptionsMenu);
    require(navigation.current == whacker::app::Screen::OptionsMenu, "push enters options");
    require(whacker::app::previous_screen_or(navigation, whacker::app::Screen::StoryHub) ==
                whacker::app::Screen::MainMenu,
        "push records previous screen");

    whacker::app::replace_screen(navigation, whacker::app::Screen::Playing);
    require(navigation.current == whacker::app::Screen::Playing, "replace updates current");
    require(whacker::app::previous_screen_or(navigation, whacker::app::Screen::StoryHub) ==
                whacker::app::Screen::MainMenu,
        "replace keeps return stack");

    require(whacker::app::pop_screen(navigation), "pop succeeds with previous screen");
    require(navigation.current == whacker::app::Screen::MainMenu, "pop returns to previous screen");
    require(!whacker::app::has_previous_screen(navigation), "stack is empty after pop");

    whacker::app::push_screen(navigation, whacker::app::Screen::StoryHub);
    whacker::app::push_screen(navigation, whacker::app::Screen::StoryScene);
    whacker::app::replace_screen(navigation, whacker::app::Screen::StoryHub);
    require(navigation.current == whacker::app::Screen::StoryHub, "replace to caller closes pushed child");
    require(whacker::app::previous_screen_or(navigation, whacker::app::Screen::StoryMenu) ==
                whacker::app::Screen::MainMenu,
        "replace to caller removes duplicate return entry");

    whacker::app::push_screen(navigation, whacker::app::Screen::StoryMenu);
    whacker::app::reset_to_root(navigation, whacker::app::Screen::MainMenu);
    require(navigation.current == whacker::app::Screen::MainMenu, "reset returns to requested root");
    require(!whacker::app::has_previous_screen(navigation), "reset clears stack");
}

void test_runtime_screen_return_contracts() {
    whacker::app::NavigationState navigation {};

    require(!whacker::app::pop_screen(navigation), "main menu back is a no-op");
    require(navigation.current == whacker::app::Screen::MainMenu, "main menu remains active after back");

    whacker::app::push_screen(navigation, whacker::app::Screen::OptionsMenu);
    require(whacker::app::pop_screen(navigation), "options back pops");
    require(navigation.current == whacker::app::Screen::MainMenu, "options returns to main menu caller");

    whacker::app::push_screen(navigation, whacker::app::Screen::QuickMatchSetup);
    require(whacker::app::pop_screen(navigation), "quick setup back pops");
    require(navigation.current == whacker::app::Screen::MainMenu, "quick setup returns to main menu caller");

    whacker::app::push_screen(navigation, whacker::app::Screen::StoryMenu);
    whacker::app::push_screen(navigation, whacker::app::Screen::StoryScene);
    require(whacker::app::pop_screen(navigation), "story scene back pops to caller");
    require(navigation.current == whacker::app::Screen::StoryMenu, "story scene returns to story menu caller");
    whacker::app::reset_to_root(navigation, whacker::app::Screen::MainMenu);

    whacker::app::push_screen(navigation, whacker::app::Screen::StoryHub);
    whacker::app::push_screen(navigation, whacker::app::Screen::StoryScene);
    require(whacker::app::pop_screen(navigation), "story scene back pops to hub caller");
    require(navigation.current == whacker::app::Screen::StoryHub, "story scene returns to story hub caller");

    whacker::app::push_screen(navigation, whacker::app::Screen::PaddleTuning);
    require(whacker::app::pop_screen(navigation), "paddle tuning commit/cancel pops");
    require(navigation.current == whacker::app::Screen::StoryHub, "paddle tuning returns to story hub caller");
    whacker::app::reset_to_root(navigation, whacker::app::Screen::MainMenu);

    whacker::app::push_screen(navigation, whacker::app::Screen::QuickMatchSetup);
    whacker::app::push_screen(navigation, whacker::app::Screen::PaddleTuning);
    require(whacker::app::pop_screen(navigation), "quick paddle tuning commit/cancel pops");
    require(
        navigation.current == whacker::app::Screen::QuickMatchSetup,
        "quick paddle tuning returns to quick setup caller");

    whacker::app::replace_screen(navigation, whacker::app::Screen::Playing);
    whacker::app::push_screen(navigation, whacker::app::Screen::Paused);
    require(whacker::app::pop_screen(navigation), "pause resume pops");
    require(navigation.current == whacker::app::Screen::Playing, "pause resume returns to active match");
}

}  // namespace

int main() {
    test_app_state_mapping();
    test_push_pop_replace_and_reset();
    test_runtime_screen_return_contracts();
    return 0;
}
