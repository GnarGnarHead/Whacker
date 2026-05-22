#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "story_portraits.hpp"
#include "story_state.hpp"

namespace whacker::app {

enum class StorySceneId : std::uint8_t {
    None = 0,
    OnboardingEarlyArrival = 1,
    OnboardingClubIntro = 2,
    OnboardingCoachBrief = 3,
    OnboardingEntryRetry = 4,
    PostForfeitSupport = 5,
    PostBenjiAtHomeYoutube = 6,
    TixMidweekLunchInvite = 7,
    TixPostLunchThanks = 8,
};

enum class StorySceneSpeaker : std::uint8_t {
    None = 0,
    Player = 1,
    Rival = 2
};

struct StorySceneState {
    StorySceneId id = StorySceneId::None;
    std::string header;
    std::array<std::string, 24> lines {};
    std::array<StorySceneSpeaker, 24> speakers {};
    std::array<StoryPortraitId, 24> portrait_ids {};
    int line_count = 0;
    int line_index = 0;
    std::size_t visible_chars = 0;
    float type_accum = 0.0f;
    int typed_line_index = 0;
    bool dialogue_writing = false;
    int scroll_lines_from_bottom = 0;
    bool has_binary_choice = false;
    bool binary_choice_yes_selected = true;
    bool player_is_right = false;
};

void clear_story_scene(StorySceneState& scene_state);
bool story_scene_has_content(const StorySceneState& scene_state);
const std::string& story_scene_current_line(const StorySceneState& scene_state);
std::string story_scene_current_line_visible(const StorySceneState& scene_state);
StorySceneSpeaker story_scene_current_speaker(const StorySceneState& scene_state);
StoryPortraitId story_scene_current_portrait(const StorySceneState& scene_state);
void reset_story_scene_typewriter(StorySceneState& scene_state);
void update_story_scene_typewriter(StorySceneState& scene_state, float dt, float speed_multiplier);
void reveal_story_scene_current_line(StorySceneState& scene_state);
bool advance_story_scene(StorySceneState& scene_state);

void begin_story_onboarding_scene(
    StorySceneState& scene_state,
    const StoryRuntimeState& story_runtime);

}  // namespace whacker::app
