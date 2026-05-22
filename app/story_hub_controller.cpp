#include "story_hub_controller.hpp"

#include "paddle_tuning_actions.hpp"
#include "story_match.hpp"
#include "story_play_session.hpp"
#include "story_runtime_invariants.hpp"

namespace whacker::app {

namespace {

bool tix_midweek_scene_pending(const StoryCareerData& career) {
    return
        career.joined_club &&
        career.tix_1967_seen &&
        !career.tix_midweek_scene_seen &&
        !career.tix_lunch_match_declined &&
        !career.tix_lunch_match_completed;
}

void save_career_if_available(
    const StoryRuntimeState& story_runtime,
    const StorySaveCareerCallback save_career_fn) {
    if (save_career_fn != nullptr) {
        (void)save_career_fn(story_runtime.career, nullptr);
    }
}

}  // namespace

StoryHubControllerEffects update_story_hub_controller(
    StoryHubControllerContext context,
    const MenuIntent& intent,
    const StorySaveCareerCallback save_career_fn) {
    StoryHubControllerEffects effects {};
    if (!context.story_runtime.career_loaded) {
        effects.route = StoryHubRoute::StoryMenu;
        return effects;
    }

    if (tix_midweek_scene_pending(context.story_runtime.career)) {
        queue_story_onboarding_scene(context.story_runtime, StoryOnboardingStep::TixMidweekScene);
        copy_onboarding_runtime_to_career(context.story_runtime);
        save_career_if_available(context.story_runtime, save_career_fn);
        begin_story_onboarding_scene(context.story_scene, context.story_runtime);
        clear_story_runtime_scene_pending_flags(context.story_runtime);
        effects.route = StoryHubRoute::StoryScene;
        return effects;
    }

    const int previous_row = context.story_hub.selected_row;
    if (intent.up) {
        context.story_hub.selected_row =
            (context.story_hub.selected_row + StoryHubRowCount - 1) % StoryHubRowCount;
    }
    if (intent.down) {
        context.story_hub.selected_row = (context.story_hub.selected_row + 1) % StoryHubRowCount;
    }
    if (context.story_hub.selected_row != previous_row) {
        effects.play_move_sound = true;
    }

    if (intent.back) {
        effects.play_confirm_sound = true;
        save_career_if_available(context.story_runtime, save_career_fn);
        effects.route = StoryHubRoute::Back;
        return effects;
    }

    if (!intent.confirm) {
        return effects;
    }
    effects.play_confirm_sound = true;

    const StoryHubRow row = static_cast<StoryHubRow>(context.story_hub.selected_row);
    if (!story_hub_row_enabled(row, context.story_runtime.career)) {
        context.story_hub.feedback_line_1 = "LOCKED FOR THIS WEEK";
        context.story_hub.feedback_line_2.clear();
        return effects;
    }

    if (row == StoryHubRowBack) {
        save_career_if_available(context.story_runtime, save_career_fn);
        effects.route = StoryHubRoute::Back;
        return effects;
    }

    if (row == StoryHubRowNextWeek) {
        advance_story_week(context.story_runtime, context.story_hub, save_career_fn);
        return effects;
    }

    if (row == StoryHubRowPaddleTuning) {
        normalize_story_player_skill_progress(
            context.story_runtime.career.player_skills,
            context.story_runtime.career.player_skill_caps);
        begin_story_player_paddle_tuning(
            context.paddle_tuning,
            context.story_runtime.career);
        effects.route = StoryHubRoute::PaddleTuning;
        return effects;
    }

    if (row == StoryHubRowOfficialMatch || row == StoryHubRowTrainingMatch) {
        const StoryMatchKind kind =
            row == StoryHubRowOfficialMatch ? StoryMatchKind::Official : StoryMatchKind::Training;
        start_story_match(
            context.story_runtime,
            context.story_hub,
            context.options,
            context.simulation,
            context.match_flow,
            context.rng,
            kind);
        effects.route = StoryHubRoute::Playing;
    }

    return effects;
}

}  // namespace whacker::app
