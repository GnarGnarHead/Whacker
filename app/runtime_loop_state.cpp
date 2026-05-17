#include "runtime_loop_state.hpp"

#ifdef WHACKER_HAS_GLFW

#include "menu_settings.hpp"
#include "runtime_helpers.hpp"

namespace whacker::app {

RuntimeLoopState::RuntimeLoopState(const std::uint64_t rng_seed)
    : rng(rng_seed) {}

void initialize_runtime_loop_state(RuntimeLoopState& loop_state) {
    load_menu_settings(loop_state.options, loop_state.controls, loop_state.audio_settings);
    loop_state.audio_settings = clamp_audio_settings(loop_state.audio_settings);
    (void)loop_state.audio_engine.init();
    loop_state.audio_engine.set_settings(loop_state.audio_settings);
}

RuntimeUpdatePhaseContext make_runtime_update_phase_context(
    GLFWwindow* window,
    RuntimeLoopState& loop_state,
    whacker::sim::Simulation& simulation) {
    return RuntimeUpdatePhaseContext {
        window,
        loop_state.edge_state,
        loop_state.app_state,
        loop_state.pause_return_state,
        loop_state.show_dev_info,
        loop_state.ai_controls_player_paddle,
        loop_state.options,
        loop_state.controls,
        loop_state.audio_settings,
        loop_state.audio_engine,
        loop_state.main_menu_state,
        loop_state.options_menu_state,
        loop_state.pause_menu_state,
        loop_state.menu_state,
        loop_state.paddle_tuning_state,
        loop_state.story_menu_state,
        loop_state.story_intro_state,
        loop_state.story_scene_state,
        loop_state.story_hub_state,
        loop_state.story_runtime,
        loop_state.visual_transition,
        loop_state.authored_transition_request,
        loop_state.match_flow,
        loop_state.left_ai_state,
        loop_state.right_ai_state,
        simulation,
        loop_state.rng,
        loop_state.type_blip_cooldown,
        loop_state.type_blip_pattern_step};
}

RuntimeRenderDriverContext make_runtime_render_driver_context(
    GLFWwindow* window,
    RuntimeLoopState& loop_state,
    const whacker::sim::Simulation& simulation) {
    return RuntimeRenderDriverContext {
        window,
        simulation,
        loop_state.match_flow,
        loop_state.show_dev_info,
        loop_state.ai_controls_player_paddle,
        loop_state.app_state,
        loop_state.pause_return_state,
        loop_state.main_menu_state,
        loop_state.options_menu_state,
        loop_state.pause_menu_state,
        loop_state.menu_state,
        loop_state.paddle_tuning_state,
        loop_state.story_menu_state,
        loop_state.story_intro_state,
        loop_state.story_scene_state,
        loop_state.story_hub_state,
        loop_state.story_runtime,
        loop_state.visual_transition,
        loop_state.options,
        loop_state.controls,
        loop_state.audio_settings,
        main_menu_row_name,
        options_menu_row_name,
        row_name,
        story_menu_row_name,
        story_intro_phase_name,
        story_hub_row_name,
        mode_name,
        ai_style_name,
        story_match_kind_name};
}

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
