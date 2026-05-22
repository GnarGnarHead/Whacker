#include "runtime_story_scene.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <string>

#include "story_script_catalog.hpp"
#include "story_match.hpp"
#include "story_runtime.hpp"
#include "story_runtime_invariants.hpp"
#include "story_save_helpers.hpp"
#include "story_text.hpp"
#include "story_transition_materialization.hpp"

namespace whacker::app {

namespace {

enum class StorySceneConfirmAction : std::uint8_t {
    StartMatch = 0,
    QueueOnboardingScene = 1,
    GoHub = 2,
};

struct StorySceneConfirmOutcome {
    StoryOnboardingStep to_step_for_wipe = StoryOnboardingStep::None;
    StorySceneConfirmAction action = StorySceneConfirmAction::StartMatch;
    StoryMatchKind match_kind = StoryMatchKind::None;
    StoryOnboardingStep queue_step = StoryOnboardingStep::None;
    Screen target_screen = Screen::StoryHub;
    bool persist_career = false;
};

using StorySceneConfirmMutateFn = void (*)(
    StoryRuntimeState& story_runtime,
    StoryHubState& story_hub_state,
    bool had_binary_choice,
    bool binary_choice_yes_selected);

struct StorySceneConfirmRule {
    StoryOnboardingStep from_step = StoryOnboardingStep::None;
    bool uses_binary_choice = false;
    StorySceneConfirmMutateFn mutate_before_action = nullptr;
    StorySceneConfirmOutcome outcome_default {};
    StorySceneConfirmOutcome outcome_yes {};
    StorySceneConfirmOutcome outcome_no {};
};

void mutate_tix_midweek_scene_choice(
    StoryRuntimeState& story_runtime,
    StoryHubState& /*story_hub_state*/,
    const bool had_binary_choice,
    const bool binary_choice_yes_selected) {
    const bool accept_lunch_match = !had_binary_choice || binary_choice_yes_selected;
    story_runtime.career.tix_midweek_scene_seen = true;
    story_runtime.career.tix_lunch_match_accepted = accept_lunch_match;
    story_runtime.career.tix_lunch_match_declined = !accept_lunch_match;
    story_runtime.career.tix_lunch_match_completed = false;
}

const StorySceneConfirmRule* story_scene_confirm_rule_for_step(const StoryOnboardingStep step) {
    static constexpr std::array<StorySceneConfirmRule, 7> kRules {{
        StorySceneConfirmRule {
            .from_step = StoryOnboardingStep::EarlyArrivalScene,
            .uses_binary_choice = false,
            .mutate_before_action = nullptr,
            .outcome_default = StorySceneConfirmOutcome {
                .to_step_for_wipe = StoryOnboardingStep::AyaFriendlyMatch,
                .action = StorySceneConfirmAction::StartMatch,
                .match_kind = StoryMatchKind::OnboardingAyaFriendly,
                .queue_step = StoryOnboardingStep::None,
                .target_screen = Screen::Playing,
                .persist_career = false,
            },
        },
        StorySceneConfirmRule {
            .from_step = StoryOnboardingStep::ClubIntroScene,
            .uses_binary_choice = false,
            .mutate_before_action = nullptr,
            .outcome_default = StorySceneConfirmOutcome {
                .to_step_for_wipe = StoryOnboardingStep::EntryBenchmarkMatch,
                .action = StorySceneConfirmAction::StartMatch,
                .match_kind = StoryMatchKind::OnboardingEntry,
                .queue_step = StoryOnboardingStep::None,
                .target_screen = Screen::Playing,
                .persist_career = false,
            },
        },
        StorySceneConfirmRule {
            .from_step = StoryOnboardingStep::CoachBriefScene,
            .uses_binary_choice = false,
            .mutate_before_action = nullptr,
            .outcome_default = StorySceneConfirmOutcome {
                .to_step_for_wipe = StoryOnboardingStep::AtHomeYoutubeScene,
                .action = StorySceneConfirmAction::QueueOnboardingScene,
                .match_kind = StoryMatchKind::None,
                .queue_step = StoryOnboardingStep::AtHomeYoutubeScene,
                .target_screen = Screen::StoryScene,
                .persist_career = true,
            },
        },
        StorySceneConfirmRule {
            .from_step = StoryOnboardingStep::AtHomeYoutubeScene,
            .uses_binary_choice = false,
            .mutate_before_action = nullptr,
            .outcome_default = StorySceneConfirmOutcome {
                .to_step_for_wipe = StoryOnboardingStep::Imagination1967Match,
                .action = StorySceneConfirmAction::StartMatch,
                .match_kind = StoryMatchKind::Imagination1967,
                .queue_step = StoryOnboardingStep::None,
                .target_screen = Screen::Playing,
                .persist_career = false,
            },
        },
        StorySceneConfirmRule {
            .from_step = StoryOnboardingStep::EntryRetryScene,
            .uses_binary_choice = false,
            .mutate_before_action = nullptr,
            .outcome_default = StorySceneConfirmOutcome {
                .to_step_for_wipe = StoryOnboardingStep::EntryBenchmarkMatch,
                .action = StorySceneConfirmAction::StartMatch,
                .match_kind = StoryMatchKind::OnboardingEntry,
                .queue_step = StoryOnboardingStep::None,
                .target_screen = Screen::Playing,
                .persist_career = false,
            },
        },
        StorySceneConfirmRule {
            .from_step = StoryOnboardingStep::TixMidweekScene,
            .uses_binary_choice = true,
            .mutate_before_action = mutate_tix_midweek_scene_choice,
            .outcome_yes = StorySceneConfirmOutcome {
                .to_step_for_wipe = StoryOnboardingStep::Complete,
                .action = StorySceneConfirmAction::StartMatch,
                .match_kind = StoryMatchKind::TixLunch,
                .queue_step = StoryOnboardingStep::None,
                .target_screen = Screen::Playing,
                .persist_career = true,
            },
            .outcome_no = StorySceneConfirmOutcome {
                .to_step_for_wipe = StoryOnboardingStep::Complete,
                .action = StorySceneConfirmAction::GoHub,
                .match_kind = StoryMatchKind::None,
                .queue_step = StoryOnboardingStep::None,
                .target_screen = Screen::StoryHub,
                .persist_career = true,
            },
        },
        StorySceneConfirmRule {
            .from_step = StoryOnboardingStep::PostTixLunchScene,
            .uses_binary_choice = false,
            .mutate_before_action = nullptr,
            .outcome_default = StorySceneConfirmOutcome {
                .to_step_for_wipe = StoryOnboardingStep::Complete,
                .action = StorySceneConfirmAction::StartMatch,
                .match_kind = StoryMatchKind::Official,
                .queue_step = StoryOnboardingStep::None,
                .target_screen = Screen::Playing,
                .persist_career = true,
            },
        },
    }};
    for (const StorySceneConfirmRule& rule : kRules) {
        if (rule.from_step == step) {
            return &rule;
        }
    }
    return nullptr;
}

void maybe_arm_story_scene_transition(
    RuntimeAuthoredTransitionRequest& authored_transition_request,
    const bool trigger_wipe,
    const Screen from_screen,
    const StorySceneState& from_story_scene_state,
    const Screen to_screen,
    const StorySceneState& current_story_scene_state,
    const StoryRuntimeState& story_runtime) {
    if (!trigger_wipe) {
        return;
    }
    const StorySceneState to_story_scene_state =
        materialize_story_scene_transition_target(current_story_scene_state, story_runtime, to_screen);
    const StorySceneState* from_scene_ptr = from_screen == Screen::StoryScene ? &from_story_scene_state : nullptr;
    const StorySceneState* to_scene_ptr = to_screen == Screen::StoryScene ? &to_story_scene_state : nullptr;
    const TransitionArmResult arm_result = arm_authored_star_wipe_transition(
        authored_transition_request,
        from_screen,
        from_scene_ptr,
        to_screen,
        to_scene_ptr);
    if (!arm_result.armed) {
#ifndef NDEBUG
        assert(false && "runtime_story_scene failed to arm authored transition request.");
#endif
        clear_authored_transition_request(authored_transition_request);
    }
}

ScreenRoute apply_story_scene_confirm_rule(
    const StorySceneConfirmOutcome& outcome,
    StoryRuntimeState& story_runtime,
    StoryHubState& story_hub_state,
    MatchOptions& options,
    MatchFlowState& match_flow,
    whacker::sim::Simulation& simulation,
    std::mt19937_64& rng,
    const StorySaveCareerCallback save_career_fn) {
    if (outcome.action == StorySceneConfirmAction::StartMatch) {
        story_runtime.onboarding_step = outcome.to_step_for_wipe;
        if (outcome.persist_career) {
            copy_onboarding_runtime_to_career(story_runtime);
            (void)persist_story_career_with_feedback(story_runtime.career, save_career_fn, &story_hub_state);
        }
        start_story_match(
            story_runtime,
            story_hub_state,
            options,
            simulation,
            match_flow,
            rng,
            outcome.match_kind);
        return screen_route(outcome.target_screen);
    }

    if (outcome.action == StorySceneConfirmAction::QueueOnboardingScene) {
        queue_story_onboarding_scene(story_runtime, outcome.queue_step);
        if (outcome.persist_career) {
            copy_onboarding_runtime_to_career(story_runtime);
            (void)persist_story_career_with_feedback(story_runtime.career, save_career_fn, &story_hub_state);
        }
        return screen_route(outcome.target_screen);
    }

    if (outcome.action == StorySceneConfirmAction::GoHub) {
        story_runtime.onboarding_step = outcome.to_step_for_wipe;
        clear_story_runtime_scene_pending_flags(story_runtime);
        if (outcome.persist_career) {
            copy_onboarding_runtime_to_career(story_runtime);
            (void)persist_story_career_with_feedback(story_runtime.career, save_career_fn, &story_hub_state);
        }
        return screen_route(outcome.target_screen);
    }
    return no_screen_route();
}

}  // namespace

StorySceneConfirmResult handle_story_scene_confirm(
    StorySceneState& story_scene_state,
    StoryRuntimeState& story_runtime,
    StoryHubState& story_hub_state,
    MatchOptions& options,
    MatchFlowState& match_flow,
    whacker::sim::Simulation& simulation,
    std::mt19937_64& rng,
    RuntimeAuthoredTransitionRequest& authored_transition_request,
    const StorySaveCareerCallback save_career_fn) {
    if (story_scene_state.dialogue_writing) {
        reveal_story_scene_current_line(story_scene_state);
        return StorySceneConfirmResult {};
    }
    constexpr Screen kFromScreen = Screen::StoryScene;
    const StorySceneState from_story_scene_state = story_scene_state;
    const bool had_binary_choice = story_scene_state.has_binary_choice;
    const bool binary_choice_yes_selected = story_scene_state.binary_choice_yes_selected;
    if (!advance_story_scene(story_scene_state)) {
        return StorySceneConfirmResult {};
    }

    if (const StorySceneConfirmRule* rule = story_scene_confirm_rule_for_step(story_runtime.onboarding_step)) {
        const bool choose_yes = !had_binary_choice || binary_choice_yes_selected;
        if (rule->mutate_before_action != nullptr) {
            rule->mutate_before_action(story_runtime, story_hub_state, had_binary_choice, binary_choice_yes_selected);
        }
        const StorySceneConfirmOutcome& outcome = rule->uses_binary_choice
            ? (choose_yes ? rule->outcome_yes : rule->outcome_no)
            : rule->outcome_default;
        const bool trigger_wipe = story_onboarding_transition_triggers_wipe(rule->from_step, outcome.to_step_for_wipe);
        const ScreenRoute route = apply_story_scene_confirm_rule(
            outcome,
            story_runtime,
            story_hub_state,
            options,
            match_flow,
            simulation,
            rng,
            save_career_fn);
        maybe_arm_story_scene_transition(
            authored_transition_request,
            trigger_wipe,
            kFromScreen,
            from_story_scene_state,
            route.changed ? route.screen : kFromScreen,
            story_scene_state,
            story_runtime);
        return StorySceneConfirmResult {.route = route};
    }
    return StorySceneConfirmResult {.route = screen_route(Screen::StoryHub)};
}

}  // namespace whacker::app
