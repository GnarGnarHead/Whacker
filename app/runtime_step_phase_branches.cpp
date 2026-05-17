#include "runtime_step_phase_internal.hpp"

#ifdef WHACKER_HAS_GLFW

#include <algorithm>
#include <cmath>

#include "match_end_flow.hpp"
#include "menu_input.hpp"
#include "paddle_tuning.hpp"
#include "play_control.hpp"
#include "runtime_helpers.hpp"
#include "sim/math.hpp"
#include "story_match.hpp"
#include "story_save.hpp"
#include "story_script_catalog.hpp"

namespace whacker::app {

namespace {

bool force_human_modes_to_ai(PaddleMode& left_mode, PaddleMode& right_mode) {
    bool changed = false;
    if (left_mode == PaddleMode::Human) {
        left_mode = PaddleMode::AI;
        changed = true;
    }
    if (right_mode == PaddleMode::Human) {
        right_mode = PaddleMode::AI;
        changed = true;
    }
    return changed;
}

whacker::progression::SkillState resolve_story_active_rival_skills(const StoryRuntimeState& story_runtime) {
    if (story_runtime.active_rival_id != StoryRivalId::None) {
        return story_runtime.active_rival_skills;
    }
    if (story_runtime.active_match == StoryMatchKind::None) {
        return {};
    }
    const StoryMatchPolicyDescriptor& policy = story_match_policy_for_kind(story_runtime.active_match);
    return story_policy_rival_spec_for_career(policy, story_runtime.career).skills;
}

void push_audio_event(AudioEngine& audio_engine, const AudioEventId event_id) {
    audio_engine.push_event(event_id);
}

void push_paddle_hit_audio(
    AudioEngine& audio_engine,
    const whacker::sim::RallyState& before,
    const whacker::sim::RallyState& after,
    const whacker::sim::SimulationConfig& config) {
    audio_engine.push_paddle_hit(build_paddle_hit_audio_params(before, after, config));
}

void push_wall_hit_audio(
    AudioEngine& audio_engine,
    const whacker::sim::RallyState& before,
    const whacker::sim::RallyState& after,
    const whacker::sim::SimulationConfig& config) {
    audio_engine.push_wall_hit(build_wall_hit_audio_params(before, after, config));
}

float next_type_blip_cooldown(std::uint32_t& type_blip_pattern_step) {
    static constexpr float kRhythmSeconds[8] = {0.040f, 0.055f, 0.043f, 0.061f, 0.046f, 0.052f, 0.041f, 0.058f};
    const std::uint32_t step = type_blip_pattern_step++;
    float cooldown = kRhythmSeconds[step % 8u];
    if (step % 9u == 4u) {
        cooldown += 0.010f;
    }
    return cooldown;
}

void update_opening_countdown_with_audio(
    MatchFlowState& match_flow,
    whacker::sim::Simulation& simulation,
    AudioEngine& audio_engine,
    const float dt_seconds) {
    const bool was_active = match_flow.opening_countdown_active;
    const bool was_visible = match_flow.opening_ball_visible;
    const float elapsed_before = match_flow.opening_countdown_elapsed;
    (void)update_match_opening_countdown(match_flow, simulation, dt_seconds);
    if (!was_active) {
        return;
    }
    const bool first_flash = elapsed_before <= 1.0e-6f && match_flow.opening_countdown_active;
    const bool visible_flash = match_flow.opening_countdown_active && !was_visible && match_flow.opening_ball_visible;
    if (first_flash || visible_flash) {
        push_audio_event(audio_engine, AudioEventId::ServeBlink);
    }
}

void finish_active_or_quick_match(RuntimeStepPhaseContext& context, const StoryMatchEndReason end_reason) {
    end_active_or_quick_match(
        context.story_runtime,
        context.story_hub_state,
        context.match_flow,
        context.simulation,
        context.story_scene_state,
        context.authored_transition_request,
        context.app_state,
        end_reason,
        context.story_official_games_to_win,
        save_story_career);
}

}  // namespace

RuntimeStepBranchOutcome step_story_intro(
    RuntimeStepPhaseContext& context,
    const RuntimeStepInputSnapshot& step_input,
    const double now,
    float& type_blip_cooldown,
    std::uint32_t& type_blip_pattern_step) {
    auto& state = context.simulation.mutable_state();
    const auto& config = context.simulation.config();
    context.story_intro_state.phase_timer += static_cast<float>(whacker::sim::kFixedDt);
    const std::size_t intro_visible_before = context.story_intro_state.visible_chars;
    update_story_intro_typewriter(
        context.story_intro_state,
        context.controls,
        static_cast<float>(whacker::sim::kFixedDt),
        step_input.text_fast_held ? 6.0f : 1.0f,
        key_name,
        sanitize_player_name);
    if (context.story_intro_state.dialogue_writing &&
        context.story_intro_state.visible_chars > intro_visible_before &&
        type_blip_cooldown <= 0.0f) {
        push_audio_event(context.audio_engine, AudioEventId::TypeBlip);
        type_blip_cooldown = next_type_blip_cooldown(type_blip_pattern_step);
    }

    if (context.story_intro_state.phase == StoryIntroPhase::PlayMatch) {
        update_opening_countdown_with_audio(
            context.match_flow,
            context.simulation,
            context.audio_engine,
            static_cast<float>(whacker::sim::kFixedDt));
        PlayControlOverrides intro_overrides {};
        intro_overrides.force_modes = true;
        intro_overrides.left_mode = context.story_intro_state.player_is_right ? PaddleMode::AI : PaddleMode::Human;
        intro_overrides.right_mode = context.story_intro_state.player_is_right ? PaddleMode::Human : PaddleMode::AI;
        intro_overrides.ai_training_context = false;
        intro_overrides.override_left_skills = true;
        intro_overrides.override_right_skills = true;
        if (context.story_intro_state.player_is_right) {
            intro_overrides.left_skills = context.story_intro_state.rival_skills;
            intro_overrides.right_skills = context.story_runtime.career.player_skills;
        } else {
            intro_overrides.left_skills = context.story_runtime.career.player_skills;
            intro_overrides.right_skills = context.story_intro_state.rival_skills;
        }
        if (context.ai_controls_player_paddle) {
            force_human_modes_to_ai(intro_overrides.left_mode, intro_overrides.right_mode);
        }
        update_targets_for_play(
            context.window,
            context.simulation,
            context.options,
            context.controls,
            context.left_ai_state,
            context.right_ai_state,
            whacker::sim::kFixedDt,
            &intro_overrides);
        const whacker::sim::RallyState before = context.simulation.state();
        const whacker::sim::ScoreEvent intro_score_event = context.simulation.step(whacker::sim::kFixedDt);
        const whacker::sim::RallyState after = context.simulation.state();
        const bool paddle_hit = after.rally_hits > before.rally_hits;
        if (paddle_hit) {
            push_paddle_hit_audio(context.audio_engine, before, after, config);
        } else if (intro_score_event == whacker::sim::ScoreEvent::None && detect_wall_bounce(before, after, config)) {
            push_wall_hit_audio(context.audio_engine, before, after, config);
        }
        if (intro_score_event != whacker::sim::ScoreEvent::None) {
            push_audio_event(context.audio_engine, AudioEventId::Score);
        }
        track_intro_contact_usage(context.story_intro_state, config, before, after);
        if (intro_score_event != whacker::sim::ScoreEvent::None) {
            const bool player_scored_point =
                context.story_intro_state.player_is_right
                ? intro_score_event == whacker::sim::ScoreEvent::RightPlayerScored
                : intro_score_event == whacker::sim::ScoreEvent::LeftPlayerScored;
            context.story_intro_state.player_scored = player_scored_point;
            context.story_intro_state.points_played += 1;

            int game_winner = 0;
            const bool game_complete = table_tennis_game_complete(after.left_score, after.right_score, &game_winner);
            if (!game_complete) {
                update_serve_after_scored_point(context.match_flow, after, context.simulation);
                if (context.story_intro_state.points_played == 1) {
                    context.story_intro_state.break_kind = StoryIntroBreak::SwapSides;
                    context.story_intro_state.swap_choice = context.story_intro_state.player_is_right ? 1 : 0;
                    context.story_intro_state.phase = StoryIntroPhase::BetweenBalls;
                    context.story_intro_state.phase_timer = 0.0f;
                    reset_story_intro_typewriter(context.story_intro_state);
                } else if (context.story_intro_state.points_played == 2 && !player_scored_point) {
                    context.story_intro_state.break_kind = StoryIntroBreak::Controls;
                    context.story_intro_state.phase = StoryIntroPhase::BetweenBalls;
                    context.story_intro_state.phase_timer = 0.0f;
                    reset_story_intro_typewriter(context.story_intro_state);
                } else if (!context.story_intro_state.name_prompted && context.story_intro_state.points_played == 3) {
                    context.story_intro_state.name_prompted = true;
                    context.story_intro_state.name_accept_pending = false;
                    context.story_intro_state.name_missing_prompt = false;
                    context.story_intro_state.phase = StoryIntroPhase::NameEntry;
                    context.story_intro_state.phase_timer = 0.0f;
                    clear_last_pressed_key();
                    reset_story_intro_typewriter(context.story_intro_state);
                } else if (!context.story_intro_state.rules_hint_shown && context.story_intro_state.points_played == 5) {
                    context.story_intro_state.rules_hint_shown = true;
                    context.story_intro_state.break_kind = StoryIntroBreak::Rules;
                    context.story_intro_state.phase = StoryIntroPhase::BetweenBalls;
                    context.story_intro_state.phase_timer = 0.0f;
                    reset_story_intro_typewriter(context.story_intro_state);
                }
            }

            if (game_complete) {
                context.story_intro_state.player_won =
                    context.story_intro_state.player_is_right ? (game_winner < 0) : (game_winner > 0);
                context.story_intro_state.player_forfeited = false;
                context.story_intro_state.final_left_score = after.left_score;
                context.story_intro_state.final_right_score = after.right_score;
                context.story_intro_state.phase = StoryIntroPhase::RivalIntro;
                context.story_intro_state.phase_timer = 0.0f;
                reset_story_intro_typewriter(context.story_intro_state);
                reset_match_flow(context.match_flow);
            }
        }
        return RuntimeStepBranchOutcome::SteppedInsideBranch;
    }

    set_paddle_execution_full(state.left);
    set_paddle_execution_full(state.right);
    const float center_y = 0.5f * config.court_height;
    const float wiggle = std::sin(static_cast<float>(now) * 6.0f) * 18.0f;
    const bool side_selected = context.story_intro_state.phase != StoryIntroPhase::Invite;
    const bool speaker_is_right = side_selected ? !context.story_intro_state.player_is_right : true;
    if (!context.story_intro_state.dialogue_writing) {
        state.left.target_y = center_y;
        state.right.target_y = center_y;
    } else if (speaker_is_right) {
        state.left.target_y = center_y;
        state.right.target_y = whacker::sim::clampf(
            center_y + wiggle,
            config.paddle_half_height,
            config.court_height - config.paddle_half_height);
    } else {
        state.left.target_y = whacker::sim::clampf(
            center_y + wiggle,
            config.paddle_half_height,
            config.court_height - config.paddle_half_height);
        state.right.target_y = center_y;
    }
    state.left.feedforward_velocity_y = 0.0f;
    state.right.feedforward_velocity_y = 0.0f;
    state.ball.position.x = 0.5f * config.court_width;
    state.ball.position.y = 0.5f * config.court_height;
    state.ball.velocity.x = 0.0f;
    state.ball.velocity.y = 0.0f;
    state.ball.spin = 0.0f;
    state.ball.speed_scalar = 1.0f;
    return RuntimeStepBranchOutcome::NeedsCommonSimulationStep;
}

RuntimeStepBranchOutcome step_story_scene(
    RuntimeStepPhaseContext& context,
    const RuntimeStepInputSnapshot& step_input,
    const double now,
    float& type_blip_cooldown,
    std::uint32_t& type_blip_pattern_step) {
    auto& state = context.simulation.mutable_state();
    const auto& config = context.simulation.config();
    const std::size_t scene_visible_before = context.story_scene_state.visible_chars;
    update_story_scene_typewriter(
        context.story_scene_state,
        static_cast<float>(whacker::sim::kFixedDt),
        step_input.text_fast_held ? 6.0f : 1.0f);
    if (context.story_scene_state.dialogue_writing &&
        context.story_scene_state.visible_chars > scene_visible_before &&
        type_blip_cooldown <= 0.0f) {
        push_audio_event(context.audio_engine, AudioEventId::TypeBlip);
        type_blip_cooldown = next_type_blip_cooldown(type_blip_pattern_step);
    }
    set_paddle_execution_full(state.left);
    set_paddle_execution_full(state.right);
    const float center_y = 0.5f * config.court_height;
    const float wiggle = std::sin(static_cast<float>(now) * 6.0f) * 18.0f;
    const bool player_is_right = context.story_runtime.career.prefers_right_side;
    bool speaker_is_right = false;
    bool has_speaking_paddle = false;
    const StorySceneSpeaker speaker = story_scene_current_speaker(context.story_scene_state);
    if (speaker == StorySceneSpeaker::Player) {
        speaker_is_right = player_is_right;
        has_speaking_paddle = true;
    } else if (speaker == StorySceneSpeaker::Rival) {
        speaker_is_right = !player_is_right;
        has_speaking_paddle = true;
    }
    if (!context.story_scene_state.dialogue_writing || !has_speaking_paddle) {
        state.left.target_y = center_y;
        state.right.target_y = center_y;
    } else if (speaker_is_right) {
        state.left.target_y = center_y;
        state.right.target_y = whacker::sim::clampf(
            center_y + wiggle,
            config.paddle_half_height,
            config.court_height - config.paddle_half_height);
    } else {
        state.left.target_y = whacker::sim::clampf(
            center_y + wiggle,
            config.paddle_half_height,
            config.court_height - config.paddle_half_height);
        state.right.target_y = center_y;
    }
    state.left.feedforward_velocity_y = 0.0f;
    state.right.feedforward_velocity_y = 0.0f;
    state.ball.position.x = 0.5f * config.court_width;
    state.ball.position.y = 0.5f * config.court_height;
    state.ball.velocity.x = 0.0f;
    state.ball.velocity.y = 0.0f;
    state.ball.spin = 0.0f;
    state.ball.speed_scalar = 1.0f;
    return RuntimeStepBranchOutcome::NeedsCommonSimulationStep;
}

RuntimeStepBranchOutcome step_non_playing_ambient(RuntimeStepPhaseContext& context) {
    update_targets_for_ambient(
        context.simulation,
        context.options,
        context.left_ai_state,
        context.right_ai_state,
        static_cast<float>(whacker::sim::kFixedDt));
    return RuntimeStepBranchOutcome::NeedsCommonSimulationStep;
}

RuntimeStepBranchOutcome step_playing(RuntimeStepPhaseContext& context) {
    update_opening_countdown_with_audio(
        context.match_flow,
        context.simulation,
        context.audio_engine,
        static_cast<float>(whacker::sim::kFixedDt));
    if (context.story_runtime.imagination_takeover_cue_seconds > 0.0f) {
        context.story_runtime.imagination_takeover_cue_seconds = std::max(
            0.0f,
            context.story_runtime.imagination_takeover_cue_seconds - static_cast<float>(whacker::sim::kFixedDt));
    }
    const whacker::sim::RallyState before = context.simulation.state();
    PlayControlOverrides overrides {};
    PlayControlOverrides* overrides_ptr = nullptr;
    StoryMatchPolicyDescriptor story_policy = story_match_policy_fallback();
    const bool has_active_story_match = context.story_runtime.active_match != StoryMatchKind::None;
    if (context.story_runtime.active_match != StoryMatchKind::None) {
        story_policy = story_match_policy_for_kind(context.story_runtime.active_match);
        const bool player_is_right = context.story_runtime.career.prefers_right_side;
        const whacker::progression::SkillState rival_skills = resolve_story_active_rival_skills(context.story_runtime);
        const int points_played = std::max(0, before.left_score + before.right_score);
        const bool ai_preview_active =
            story_policy.ai_preview_points > 0 && points_played < story_policy.ai_preview_points;
        if (context.story_runtime.active_match == StoryMatchKind::Imagination1967 &&
            !ai_preview_active &&
            !context.story_runtime.imagination_takeover_cue_shown) {
            context.story_runtime.imagination_takeover_cue_shown = true;
            context.story_runtime.imagination_takeover_cue_seconds = 1.15f;
        }
        overrides.force_modes = true;
        if (ai_preview_active) {
            overrides.left_mode = PaddleMode::AI;
            overrides.right_mode = PaddleMode::AI;
        } else {
            overrides.left_mode = player_is_right ? PaddleMode::AI : PaddleMode::Human;
            overrides.right_mode = player_is_right ? PaddleMode::Human : PaddleMode::AI;
        }
        overrides.ai_training_context = story_policy.ai_training_context;
        overrides.override_left_skills = true;
        overrides.override_right_skills = true;
        if (context.story_runtime.active_match == StoryMatchKind::Imagination1967) {
            const whacker::progression::SkillState champion_player = story_script_imagination_1967_player_skills();
            const whacker::progression::SkillState champion_rival = story_script_imagination_1967_rival_skills();
            if (player_is_right) {
                overrides.left_skills = champion_rival;
                overrides.right_skills = champion_player;
            } else {
                overrides.left_skills = champion_player;
                overrides.right_skills = champion_rival;
            }
        } else {
            if (player_is_right) {
                overrides.left_skills = rival_skills;
                overrides.right_skills = context.story_runtime.career.player_skills;
            } else {
                overrides.left_skills = context.story_runtime.career.player_skills;
                overrides.right_skills = rival_skills;
            }
        }
        overrides_ptr = &overrides;
    }
    if (context.ai_controls_player_paddle) {
        if (overrides_ptr != nullptr) {
            force_human_modes_to_ai(overrides.left_mode, overrides.right_mode);
        } else {
            PaddleMode left_mode = context.options.left_mode;
            PaddleMode right_mode = context.options.right_mode;
            if (force_human_modes_to_ai(left_mode, right_mode)) {
                overrides.force_modes = true;
                overrides.left_mode = left_mode;
                overrides.right_mode = right_mode;
                overrides_ptr = &overrides;
            }
        }
    }
    update_targets_for_play(
        context.window,
        context.simulation,
        context.options,
        context.controls,
        context.left_ai_state,
        context.right_ai_state,
        whacker::sim::kFixedDt,
        overrides_ptr);
    const whacker::sim::ScoreEvent score_event = context.simulation.step(whacker::sim::kFixedDt);
    const whacker::sim::RallyState after = context.simulation.state();
    const bool paddle_hit = after.rally_hits > before.rally_hits;
    if (paddle_hit) {
        push_paddle_hit_audio(context.audio_engine, before, after, context.simulation.config());
    } else if (score_event == whacker::sim::ScoreEvent::None &&
               detect_wall_bounce(before, after, context.simulation.config())) {
        push_wall_hit_audio(context.audio_engine, before, after, context.simulation.config());
    }
    if (score_event != whacker::sim::ScoreEvent::None) {
        push_audio_event(context.audio_engine, AudioEventId::Score);
    }
    if (has_active_story_match) {
        update_story_match_tracking(
            context.story_runtime,
            context.simulation.config(),
            before,
            after,
            whacker::sim::kFixedDt);

        if (score_event != whacker::sim::ScoreEvent::None) {
            switch (story_policy.score_model) {
                case StoryMatchScoreModel::BestOfGames: {
                    int game_winner = 0;
                    if (table_tennis_game_complete(after.left_score, after.right_score, &game_winner)) {
                        if (game_winner > 0) {
                            context.story_runtime.official_games_left += 1;
                        } else if (game_winner < 0) {
                            context.story_runtime.official_games_right += 1;
                        }

                        if (context.story_runtime.official_games_left >= context.story_official_games_to_win ||
                            context.story_runtime.official_games_right >= context.story_official_games_to_win) {
                            finish_active_or_quick_match(context, StoryMatchEndReason::Completed);
                        } else {
                            start_next_table_tennis_game(context.match_flow, context.simulation, true);
                        }
                    } else {
                        update_serve_after_scored_point(context.match_flow, after, context.simulation);
                    }
                    break;
                }
                case StoryMatchScoreModel::RallyLoop:
                    update_serve_after_scored_point(context.match_flow, after, context.simulation);
                    break;
                case StoryMatchScoreModel::SingleGame:
                case StoryMatchScoreModel::None:
                default: {
                    int game_winner = 0;
                    if (table_tennis_game_complete(after.left_score, after.right_score, &game_winner)) {
                        finish_active_or_quick_match(context, StoryMatchEndReason::Completed);
                    } else {
                        update_serve_after_scored_point(context.match_flow, after, context.simulation);
                    }
                    break;
                }
            }
        }
    } else if (score_event != whacker::sim::ScoreEvent::None) {
        int game_winner = 0;
        if (table_tennis_game_complete(after.left_score, after.right_score, &game_winner)) {
            finish_active_or_quick_match(context, StoryMatchEndReason::Completed);
        } else {
            update_serve_after_scored_point(context.match_flow, after, context.simulation);
        }
    }
    return RuntimeStepBranchOutcome::SteppedInsideBranch;
}

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
