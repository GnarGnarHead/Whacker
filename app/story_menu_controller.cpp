#include "story_menu_controller.hpp"

#include "story_continue_resume.hpp"
#include "story_menu_actions.hpp"
#include "story_runtime_invariants.hpp"
#include "story_scene.hpp"
#include "story_text.hpp"

namespace whacker::app {

namespace {

void set_story_menu_feedback(StoryMenuControllerContext& context, const std::string& text) {
    if (context.feedback != nullptr) {
        *context.feedback = text;
    }
}

void clear_story_menu_feedback(StoryMenuControllerContext& context) {
    if (context.feedback != nullptr) {
        context.feedback->clear();
    }
}

void continue_loaded_story(
    StoryMenuControllerContext& context,
    const StoryCareerData& loaded,
    StoryMenuControllerEffects& effects) {
    StoryRuntimeState next_story_runtime = context.story_runtime;
    const AppState loaded_state = apply_continue_loaded_career(next_story_runtime, loaded);
    context.story_runtime = next_story_runtime;
    if (loaded_state == AppState::StoryHub) {
        context.story_hub.selected_row = StoryHubRowOfficialMatch;
        context.story_hub.feedback_line_1 = story_text::career_loaded_feedback_line_1();
        context.story_hub.feedback_line_2 =
            story_text::career_loaded_feedback_line_2(context.story_runtime.career.week);
        effects.route = StoryMenuRoute::StoryHub;
        return;
    }

    begin_story_onboarding_scene(context.story_scene, context.story_runtime);
    clear_story_runtime_scene_pending_flags(context.story_runtime);
    effects.route = StoryMenuRoute::StoryScene;
}

}  // namespace

StoryMenuControllerEffects update_story_menu_controller(
    StoryMenuControllerContext context,
    const ActionInputFrame& input,
    const bool has_save,
    const StoryLoadCareerCallback load_career_fn,
    const StoryResetCareerFn reset_career_fn) {
    StoryMenuControllerEffects effects {};
    const int previous_row = context.menu.selected_row;
    const bool confirm_before = context.menu.confirm_overwrite;
    const int confirm_selected_before = context.menu.confirm_selected;

    const StoryMenuActionResult result = apply_story_menu_action_frame(context.menu, has_save, input);
    if (context.menu.selected_row != previous_row) {
        clear_story_menu_feedback(context);
        effects.play_move_sound = true;
    }
    if (context.menu.confirm_selected != confirm_selected_before) {
        effects.play_move_sound = true;
    }
    if (result != StoryMenuActionResult::None || context.menu.confirm_overwrite != confirm_before) {
        effects.play_confirm_sound = true;
    }

    if (result == StoryMenuActionResult::Back) {
        effects.route = StoryMenuRoute::MainMenu;
        return effects;
    }

    if (result == StoryMenuActionResult::Continue) {
        StoryCareerData loaded {};
        std::string load_error;
        if (load_career_fn == nullptr || !load_career_fn(loaded, &load_error)) {
            set_story_menu_feedback(
                context,
                load_error.empty() ? "COULD NOT LOAD STORY SAVE" : load_error);
            return effects;
        }
        clear_story_menu_feedback(context);
        continue_loaded_story(context, loaded, effects);
        return effects;
    }

    if (result == StoryMenuActionResult::NewCareer) {
        context.menu.confirm_overwrite = false;
        context.menu.confirm_selected = 0;
        clear_story_menu_feedback(context);
        begin_new_story_intro(
            context.story_runtime,
            context.story_hub,
            context.story_intro,
            context.options,
            context.match_flow,
            context.simulation,
            reset_career_fn);
        effects.route = StoryMenuRoute::StoryIntro;
    }

    return effects;
}

}  // namespace whacker::app
