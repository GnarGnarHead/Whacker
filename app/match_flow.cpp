#include "match_flow.hpp"

namespace whacker::app {

namespace {

constexpr float kMatchStartBlinkHalfPeriod = 0.20f;
constexpr int kMatchStartBlinkCount = 3;

void hold_ball_center(whacker::sim::Simulation& simulation) {
    auto& state = simulation.mutable_state();
    const auto& config = simulation.config();
    state.ball.position.x = 0.5f * config.court_width;
    state.ball.position.y = 0.5f * config.court_height;
    state.ball.velocity.x = 0.0f;
    state.ball.velocity.y = 0.0f;
    state.ball.spin = 0.0f;
    state.ball.speed_scalar = 1.0f;
}

void begin_opening_countdown(MatchFlowState& match_flow, whacker::sim::Simulation& simulation) {
    match_flow.opening_countdown_active = true;
    match_flow.opening_countdown_elapsed = 0.0f;
    match_flow.opening_ball_visible = true;
    hold_ball_center(simulation);
}

}  // namespace

bool randomize_opening_serve(whacker::sim::Simulation& simulation, std::mt19937_64& rng) {
    std::uniform_int_distribution<int> side_pick(0, 1);
    const bool serve_to_right = side_pick(rng) == 1;
    auto& state = simulation.mutable_state();
    const auto& config = simulation.config();
    state.ball.velocity.x = serve_to_right ? config.ball_base_speed : -config.ball_base_speed;
    return serve_to_right;
}

void set_serve_direction(whacker::sim::Simulation& simulation, const bool serve_to_right) {
    auto& state = simulation.mutable_state();
    const auto& config = simulation.config();
    state.ball.velocity.x = serve_to_right ? config.ball_base_speed : -config.ball_base_speed;
}

bool table_tennis_game_complete(const int left_score, const int right_score, int* winner_out) {
    const int diff = left_score - right_score;
    const bool left_won = left_score >= kTableTennisPointsToWin && diff >= kTableTennisWinBy;
    const bool right_won = right_score >= kTableTennisPointsToWin && diff <= -kTableTennisWinBy;
    if (winner_out != nullptr) {
        *winner_out = left_won ? 1 : (right_won ? -1 : 0);
    }
    return left_won || right_won;
}

bool table_tennis_deuce_serve_mode(
    const int left_score,
    const int right_score,
    const bool use_deuce_serve) {
    return
        use_deuce_serve &&
        left_score >= (kTableTennisPointsToWin - 1) &&
        right_score >= (kTableTennisPointsToWin - 1);
}

int table_tennis_serves_before_switch(
    const int left_score,
    const int right_score,
    const bool use_deuce_serve) {
    return table_tennis_deuce_serve_mode(left_score, right_score, use_deuce_serve)
        ? kTableTennisDeuceServeBlock
        : kTableTennisServeBlock;
}

void reset_match_flow(MatchFlowState& match_flow) {
    match_flow = MatchFlowState {};
}

void start_match_flow(
    MatchFlowState& match_flow,
    const ActiveMatchMode mode,
    const bool opening_serve_to_right,
    const bool use_deuce_serve) {
    match_flow.mode = mode;
    match_flow.use_deuce_serve = use_deuce_serve;
    match_flow.opening_serve_to_right = opening_serve_to_right;
    match_flow.serve_to_right = opening_serve_to_right;
    match_flow.serves_by_current_server = 0;
    match_flow.opening_countdown_active = false;
    match_flow.opening_countdown_elapsed = 0.0f;
    match_flow.opening_ball_visible = true;
}

void start_match_opening_countdown(MatchFlowState& match_flow, whacker::sim::Simulation& simulation) {
    begin_opening_countdown(match_flow, simulation);
}

void update_serve_after_scored_point(
    MatchFlowState& match_flow,
    const whacker::sim::RallyState& score_state,
    whacker::sim::Simulation& simulation) {
    if (match_flow.mode == ActiveMatchMode::None) {
        return;
    }
    const int serves_before_switch =
        table_tennis_serves_before_switch(
            score_state.left_score,
            score_state.right_score,
            match_flow.use_deuce_serve);

    match_flow.serves_by_current_server += 1;
    if (match_flow.serves_by_current_server >= serves_before_switch) {
        match_flow.serves_by_current_server = 0;
        match_flow.serve_to_right = !match_flow.serve_to_right;
    }

    begin_opening_countdown(match_flow, simulation);
}

void start_next_table_tennis_game(
    MatchFlowState& match_flow,
    whacker::sim::Simulation& simulation,
    const bool alternate_opener) {
    if (alternate_opener) {
        match_flow.opening_serve_to_right = !match_flow.opening_serve_to_right;
    }
    match_flow.serve_to_right = match_flow.opening_serve_to_right;
    match_flow.serves_by_current_server = 0;
    simulation.reset();
    begin_opening_countdown(match_flow, simulation);
}

bool update_match_opening_countdown(
    MatchFlowState& match_flow,
    whacker::sim::Simulation& simulation,
    const float dt) {
    if (!match_flow.opening_countdown_active) {
        return false;
    }

    hold_ball_center(simulation);

    const float safe_dt = dt > 0.0f ? dt : 0.0f;
    match_flow.opening_countdown_elapsed += safe_dt;
    const int phase = static_cast<int>(match_flow.opening_countdown_elapsed / kMatchStartBlinkHalfPeriod);
    const int total_phases = kMatchStartBlinkCount * 2;
    if (phase >= total_phases) {
        match_flow.opening_countdown_active = false;
        match_flow.opening_ball_visible = true;
        set_serve_direction(simulation, match_flow.serve_to_right);
        return true;
    }

    match_flow.opening_ball_visible = (phase % 2) == 0;
    return false;
}

bool match_opening_ball_visible(const MatchFlowState& match_flow) {
    if (!match_flow.opening_countdown_active) {
        return true;
    }
    return match_flow.opening_ball_visible;
}

}  // namespace whacker::app
