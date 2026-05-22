#include <cassert>

#include "runtime_visual_transition.hpp"

namespace {

void test_state_change_transition_swaps_at_midpoint_and_completes() {
    whacker::app::RuntimeVisualTransitionState transition {};
    whacker::app::StorySceneState scene_state {};

    whacker::app::begin_visual_transition_for_state_change(
        transition,
        whacker::app::Screen::StoryHub,
        whacker::app::Screen::StoryScene,
        10.0,
        0.45f);

    assert(transition.active);

    whacker::app::ScreenRoute route =
        whacker::app::advance_visual_transition(transition, scene_state, 10.10);
    assert(transition.active);
    assert(!route.changed);

    route = whacker::app::advance_visual_transition(transition, scene_state, 10.30);
    assert(transition.active);
    assert(route.changed);
    assert(route.screen == whacker::app::Screen::StoryScene);

    route = whacker::app::advance_visual_transition(transition, scene_state, 10.60);
    assert(!transition.active);
    assert(!route.changed);
}

void test_scene_change_transition_swaps_story_scene_payload_at_midpoint() {
    whacker::app::RuntimeVisualTransitionState transition {};
    whacker::app::StorySceneState from_scene {};
    whacker::app::StorySceneState to_scene {};
    from_scene.id = whacker::app::StorySceneId::OnboardingClubIntro;
    from_scene.header = "FROM";
    to_scene.id = whacker::app::StorySceneId::OnboardingCoachBrief;
    to_scene.header = "TO";
    whacker::app::StorySceneState live_scene = from_scene;

    whacker::app::begin_visual_transition_for_scene_change(transition, from_scene, to_scene, 21.0, 0.45f);
    assert(transition.active);
    assert(live_scene.id == from_scene.id);
    assert(live_scene.header == "FROM");

    whacker::app::ScreenRoute route =
        whacker::app::advance_visual_transition(transition, live_scene, 21.10);
    assert(!route.changed);
    assert(live_scene.id == from_scene.id);
    assert(live_scene.header == "FROM");

    route = whacker::app::advance_visual_transition(transition, live_scene, 21.30);
    assert(!route.changed);
    assert(live_scene.id == to_scene.id);
    assert(live_scene.header == "TO");

    route = whacker::app::advance_visual_transition(transition, live_scene, 21.60);
    assert(!route.changed);
    assert(!transition.active);
    assert(live_scene.id == to_scene.id);
    assert(live_scene.header == "TO");
}

}  // namespace

int main() {
    test_state_change_transition_swaps_at_midpoint_and_completes();
    test_scene_change_transition_swaps_story_scene_payload_at_midpoint();
    return 0;
}
