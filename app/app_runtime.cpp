#include "app_runtime.hpp"

#include <algorithm>
#include <string>

#include "action_input.hpp"
#include "game_render.hpp"
#include "main_menu_actions.hpp"
#include "menu_overlay.hpp"
#include "play_control_human.hpp"
#include "platform_sdl.hpp"
#include "sdl_input.hpp"
#include "ui_state.hpp"

namespace whacker::app {

namespace {

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

struct SdlRuntimeState {
    AppState app_state = AppState::MainMenu;
    MainMenuState main_menu {};
    std::string main_menu_feedback {};
    SdlKeyboardInput keyboard {};
    double previous_time = 0.0;
    double accumulator = 0.0;
};

void start_quick_match(SdlRuntimeState& runtime, whacker::sim::Simulation& simulation) {
    simulation.reset();
    runtime.main_menu_feedback.clear();
    runtime.app_state = AppState::Playing;
}

void apply_main_menu_result(
    const MainMenuActionResult result,
    SdlRuntimeState& runtime,
    whacker::sim::Simulation& simulation,
    SdlPlatform& platform) {
    switch (result) {
        case MainMenuActionResult::None:
            return;
        case MainMenuActionResult::Story:
            runtime.main_menu_feedback = "STORY MODE NOT WIRED YET";
            return;
        case MainMenuActionResult::Quick:
            start_quick_match(runtime, simulation);
            return;
        case MainMenuActionResult::Options:
            runtime.main_menu_feedback = "OPTIONS NOT WIRED YET";
            return;
        case MainMenuActionResult::Quit:
            platform.request_close();
            return;
    }
}

void update_main_menu(
    const ActionInputFrame& input,
    SdlRuntimeState& runtime,
    whacker::sim::Simulation& simulation,
    SdlPlatform& platform) {
    const int previous_row = runtime.main_menu.selected_row;
    const MainMenuActionResult result = apply_main_menu_action_frame(runtime.main_menu, input);
    if (runtime.main_menu.selected_row != previous_row) {
        runtime.main_menu_feedback.clear();
    }
    apply_main_menu_result(result, runtime, simulation, platform);
    runtime.accumulator = 0.0;
}

void step_playing_keyboard(
    const ActionInputFrame& input,
    whacker::sim::Simulation& simulation) {
    auto& state = simulation.mutable_state();
    const auto& config = simulation.config();
    set_human_axis_target(state.left, config, input.p1_move_y, whacker::sim::kFixedDt);
    set_human_axis_target(state.right, config, input.p2_move_y, whacker::sim::kFixedDt);
    (void)simulation.step(whacker::sim::kFixedDt);
}

void update_playing(
    const ActionInputFrame& input,
    const double frame_dt,
    SdlRuntimeState& runtime,
    whacker::sim::Simulation& simulation) {
    if (input_pressed(input, InputAction::Pause)) {
        runtime.app_state = AppState::Paused;
        runtime.accumulator = 0.0;
        return;
    }

    runtime.accumulator = std::min(runtime.accumulator + frame_dt, 0.25);
    while (runtime.accumulator >= static_cast<double>(whacker::sim::kFixedDt)) {
        step_playing_keyboard(input, simulation);
        runtime.accumulator -= static_cast<double>(whacker::sim::kFixedDt);
    }
}

void update_paused(const ActionInputFrame& input, SdlRuntimeState& runtime) {
    if (input_pressed(input, InputAction::Pause) ||
        input_pressed(input, InputAction::Confirm) ||
        input_pressed(input, InputAction::Back)) {
        runtime.app_state = AppState::Playing;
    }
    runtime.accumulator = 0.0;
}

void update_runtime(
    const ActionInputFrame& input,
    const double frame_dt,
    SdlRuntimeState& runtime,
    whacker::sim::Simulation& simulation,
    SdlPlatform& platform) {
    if (runtime.app_state == AppState::MainMenu) {
        update_main_menu(input, runtime, simulation, platform);
    } else if (runtime.app_state == AppState::Playing) {
        update_playing(input, frame_dt, runtime, simulation);
    } else if (runtime.app_state == AppState::Paused) {
        update_paused(input, runtime);
    }
}

void render_runtime_frame(
    const RenderContext& render_context,
    const SdlRuntimeState& runtime,
    const whacker::sim::Simulation& simulation) {
    render_scene(render_context, simulation, true);
    if (runtime.app_state == AppState::MainMenu) {
        render_main_menu_overlay(
            render_context,
            runtime.main_menu,
            main_menu_row_name,
            runtime.main_menu_feedback);
    } else {
        render_hud(render_context, simulation);
        if (runtime.app_state == AppState::Paused) {
            render_play_center_message(render_context, "PAUSED");
        }
    }
}

}  // namespace

int run_app_loop(SdlPlatform& platform, whacker::sim::Simulation& simulation) {
    SdlRuntimeState runtime {};
    runtime.previous_time = platform.now_seconds();

    while (!platform.should_close()) {
        platform.poll_events();
        runtime.keyboard.sample();
        const ActionInputFrame& input = runtime.keyboard.frame();

        const double now = platform.now_seconds();
        const double frame_dt = std::clamp(now - runtime.previous_time, 0.0, 0.1);
        runtime.previous_time = now;

        update_runtime(input, frame_dt, runtime, simulation, platform);
        render_runtime_frame(platform.render_context(), runtime, simulation);
        platform.swap_buffers();
    }

    return 0;
}

}  // namespace whacker::app
