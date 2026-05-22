#include "sdl_story_hub_rules.hpp"

#include "story_script_catalog.hpp"

namespace whacker::app {

bool sdl_story_hub_row_enabled(const StoryHubRow row, const StoryCareerData& career) {
    switch (row) {
        case StoryHubRowOfficialMatch:
            return career.joined_club && !career.official_completed && !career.story_completed;
        case StoryHubRowTrainingMatch:
            return career.joined_club && !career.story_completed;
        case StoryHubRowNextWeek:
            return
                career.joined_club &&
                career.official_completed &&
                !career.story_completed &&
                story_graph_has_next_node(career);
        case StoryHubRowPaddleTuning:
            return true;
        case StoryHubRowBack:
            return true;
        default:
            return false;
    }
}

}  // namespace whacker::app
