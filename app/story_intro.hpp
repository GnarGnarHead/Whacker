#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

#include "app_types.hpp"
#include "menu_input.hpp"
#include "progression/skills.hpp"
#include "story_state.hpp"

#ifdef WHACKER_HAS_GLFW

namespace whacker::app {

enum class StoryIntroPhase : std::uint8_t {
    Invite = 0,
    PlayMatch = 1,
    BetweenBalls = 2,
    NameEntry = 3,
    RivalIntro = 4
};

enum class StoryIntroBreak : std::uint8_t {
    None = 0,
    SwapSides = 1,
    Controls = 2,
    Rules = 3
};

struct StoryIntroState {
    StoryIntroPhase phase = StoryIntroPhase::Invite;
    StoryIntroBreak break_kind = StoryIntroBreak::None;
    int swap_choice = 0;
    bool player_is_right = false;
    bool name_prompted = false;
    bool name_accept_pending = false;
    bool name_missing_prompt = false;
    bool rules_hint_shown = false;
    bool player_scored = false;
    bool player_won = false;
    bool player_forfeited = false;
    bool dialogue_writing = false;
    int points_played = 0;
    int final_left_score = 0;
    int final_right_score = 0;
    whacker::progression::SkillUsageAccumulator player_usage {};
    std::size_t visible_chars = 0;
    float type_accum = 0.0f;
    int scroll_lines_from_bottom = 0;
    float phase_timer = 0.0f;
    StoryIntroPhase typed_phase = StoryIntroPhase::Invite;
    StoryIntroBreak typed_break = StoryIntroBreak::None;
    std::string entered_name;
    StoryRivalId rival_id = StoryRivalId::Kai;
    std::string rival_name = "KAI";
    AiStyle rival_style = AiStyle::Balanced;
    whacker::progression::SkillState rival_skills {.edge = 0.16f, .power = 0.16f, .spin_inject = 0.16f};
};

struct StoryIntroDialogue {
    std::string header;
    std::string line_1;
    std::string line_2;
    std::string line_3;
    std::array<std::string, 2> options {};
    int option_count = 0;
};

using StoryIntroSanitizeNameFn = std::string (*)(const std::string&);
using StoryIntroKeyNameFn = const char* (*)(int);

void reset_story_intro_typewriter(StoryIntroState& story_intro_state);
void reveal_story_intro_typewriter(StoryIntroState& story_intro_state);
StoryIntroDialogue compose_story_intro_dialogue(
    const StoryIntroState& story_intro_state,
    const ControlBindings& controls,
    StoryIntroKeyNameFn key_name_fn,
    StoryIntroSanitizeNameFn sanitize_name_fn);
std::size_t story_intro_dialogue_char_count(const StoryIntroDialogue& dialogue);
void update_story_intro_typewriter(
    StoryIntroState& story_intro_state,
    const ControlBindings& controls,
    float dt,
    float speed_multiplier,
    StoryIntroKeyNameFn key_name_fn,
    StoryIntroSanitizeNameFn sanitize_name_fn);

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
