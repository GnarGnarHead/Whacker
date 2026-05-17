#pragma once

#include <cstdint>
#include <random>

#include "sim/physics.hpp"

namespace whacker::app {

constexpr int kTableTennisPointsToWin = 11;
constexpr int kTableTennisWinBy = 2;
constexpr int kTableTennisServeBlock = 2;
constexpr int kTableTennisDeuceServeBlock = 1;

enum class ActiveMatchMode : std::uint8_t {
    None = 0,
    Quick = 1,
    StoryTraining = 2,
    StoryOfficial = 3
};

struct MatchFlowState {
    ActiveMatchMode mode = ActiveMatchMode::None;
    bool use_deuce_serve = false;
    bool opening_serve_to_right = true;
    bool serve_to_right = true;
    int serves_by_current_server = 0;
    bool opening_countdown_active = false;
    float opening_countdown_elapsed = 0.0f;
    bool opening_ball_visible = true;
};

bool randomize_opening_serve(whacker::sim::Simulation& simulation, std::mt19937_64& rng);
void set_serve_direction(whacker::sim::Simulation& simulation, bool serve_to_right);
bool table_tennis_game_complete(int left_score, int right_score, int* winner_out = nullptr);
bool table_tennis_deuce_serve_mode(int left_score, int right_score, bool use_deuce_serve);
int table_tennis_serves_before_switch(int left_score, int right_score, bool use_deuce_serve);
void reset_match_flow(MatchFlowState& match_flow);
void start_match_flow(
    MatchFlowState& match_flow,
    ActiveMatchMode mode,
    bool opening_serve_to_right,
    bool use_deuce_serve);
void start_match_opening_countdown(MatchFlowState& match_flow, whacker::sim::Simulation& simulation);
void update_serve_after_scored_point(
    MatchFlowState& match_flow,
    const whacker::sim::RallyState& score_state,
    whacker::sim::Simulation& simulation);
void start_next_table_tennis_game(
    MatchFlowState& match_flow,
    whacker::sim::Simulation& simulation,
    bool alternate_opener);
bool update_match_opening_countdown(
    MatchFlowState& match_flow,
    whacker::sim::Simulation& simulation,
    float dt);
bool match_opening_ball_visible(const MatchFlowState& match_flow);

}  // namespace whacker::app
