#include "story_play_session.hpp"

namespace whacker::app {

void start_story_play_session(
    MatchOptions& options,
    whacker::sim::Simulation& simulation,
    MatchFlowState& match_flow,
    std::mt19937_64& rng,
    const ActiveMatchMode mode,
    const bool player_is_right,
    const AiStyle rival_style,
    const whacker::progression::SkillState& rival_skills,
    const whacker::progression::SkillState& player_skills) {
    options.left_mode = player_is_right ? PaddleMode::AI : PaddleMode::Human;
    options.right_mode = player_is_right ? PaddleMode::Human : PaddleMode::AI;
    options.left_ai_style = player_is_right ? rival_style : AiStyle::Balanced;
    options.right_ai_style = player_is_right ? AiStyle::Balanced : rival_style;
    options.left_paddle_skills = player_is_right ? rival_skills : player_skills;
    options.right_paddle_skills = player_is_right ? player_skills : rival_skills;

    simulation.reset();
    const bool opening_serve_to_right = randomize_opening_serve(simulation, rng);
    start_match_flow(match_flow, mode, opening_serve_to_right, true);
    start_match_opening_countdown(match_flow, simulation);
}

}  // namespace whacker::app
