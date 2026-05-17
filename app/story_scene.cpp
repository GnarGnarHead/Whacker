#include "story_scene.hpp"

#ifdef WHACKER_HAS_GLFW

#include <algorithm>

#include "story_script_catalog.hpp"

namespace whacker::app {

namespace {

int current_line_index(const StorySceneState& scene_state) {
    return std::clamp(scene_state.line_index, 0, scene_state.line_count - 1);
}

const std::string& current_line(const StorySceneState& scene_state) {
    return scene_state.lines[static_cast<std::size_t>(current_line_index(scene_state))];
}

}  // namespace

void clear_story_scene(StorySceneState& scene_state) {
    scene_state = StorySceneState {};
}

bool story_scene_has_content(const StorySceneState& scene_state) {
    return scene_state.id != StorySceneId::None && scene_state.line_count > 0;
}

const std::string& story_scene_current_line(const StorySceneState& scene_state) {
    static const std::string kEmpty;
    if (!story_scene_has_content(scene_state)) {
        return kEmpty;
    }
    return current_line(scene_state);
}

std::string story_scene_current_line_visible(const StorySceneState& scene_state) {
    if (!story_scene_has_content(scene_state)) {
        return {};
    }
    const std::string& full = current_line(scene_state);
    if (scene_state.visible_chars >= full.size()) {
        return full;
    }
    return full.substr(0, scene_state.visible_chars);
}

StorySceneSpeaker story_scene_current_speaker(const StorySceneState& scene_state) {
    if (!story_scene_has_content(scene_state)) {
        return StorySceneSpeaker::None;
    }
    return scene_state.speakers[static_cast<std::size_t>(current_line_index(scene_state))];
}

StoryPortraitId story_scene_current_portrait(const StorySceneState& scene_state) {
    if (!story_scene_has_content(scene_state)) {
        return StoryPortraitId::None;
    }
    return scene_state.portrait_ids[static_cast<std::size_t>(current_line_index(scene_state))];
}

void reset_story_scene_typewriter(StorySceneState& scene_state) {
    scene_state.visible_chars = 0;
    scene_state.type_accum = 0.0f;
    scene_state.typed_line_index = scene_state.line_index;
    scene_state.dialogue_writing = story_scene_has_content(scene_state);
    scene_state.scroll_lines_from_bottom = 0;
}

void update_story_scene_typewriter(
    StorySceneState& scene_state,
    const float dt,
    const float speed_multiplier) {
    if (!story_scene_has_content(scene_state)) {
        scene_state.dialogue_writing = false;
        scene_state.scroll_lines_from_bottom = 0;
        return;
    }
    if (scene_state.typed_line_index != scene_state.line_index) {
        reset_story_scene_typewriter(scene_state);
    }

    const std::string& line = current_line(scene_state);
    if (line.empty()) {
        scene_state.visible_chars = 0;
        scene_state.dialogue_writing = false;
        scene_state.scroll_lines_from_bottom = 0;
        return;
    }

    constexpr float kStorySceneCharsPerSecond = 30.0f;
    const float capped_multiplier = std::clamp(speed_multiplier, 1.0f, 12.0f);
    if (scene_state.visible_chars >= line.size()) {
        scene_state.visible_chars = line.size();
        scene_state.dialogue_writing = false;
        return;
    }
    scene_state.type_accum += std::max(0.0f, dt) * kStorySceneCharsPerSecond * capped_multiplier;
    const std::size_t add_chars = static_cast<std::size_t>(scene_state.type_accum);
    if (add_chars > 0) {
        scene_state.visible_chars = std::min(line.size(), scene_state.visible_chars + add_chars);
        scene_state.type_accum -= static_cast<float>(add_chars);
    }
    scene_state.dialogue_writing = scene_state.visible_chars < line.size();
    if (scene_state.dialogue_writing) {
        scene_state.scroll_lines_from_bottom = 0;
    }
}

void reveal_story_scene_current_line(StorySceneState& scene_state) {
    if (!story_scene_has_content(scene_state)) {
        return;
    }
    const std::string& line = current_line(scene_state);
    scene_state.visible_chars = line.size();
    scene_state.type_accum = 0.0f;
    scene_state.dialogue_writing = false;
    scene_state.scroll_lines_from_bottom = 0;
}

bool advance_story_scene(StorySceneState& scene_state) {
    if (!story_scene_has_content(scene_state)) {
        return true;
    }
    scene_state.line_index += 1;
    if (scene_state.line_index >= scene_state.line_count) {
        clear_story_scene(scene_state);
        return true;
    }
    reset_story_scene_typewriter(scene_state);
    return false;
}

void begin_story_onboarding_scene(
    StorySceneState& scene_state,
    const StoryRuntimeState& story_runtime) {
    populate_story_onboarding_scene_script(scene_state, story_runtime);
}

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
