#include "ui_text.hpp"

#include <string>
#include <string_view>

namespace {

constexpr char kUnknownLabel[] = "?";
constexpr char kSelectedMarker[] = ">";
constexpr char kUnselectedMarker[] = " ";

constexpr char kMainMenuTitle[] = "WHACKER";
constexpr char kMainMenuSubtitle[] = "SELECT MODE";
constexpr char kMainMenuFooter[] = "UP/DOWN MOVE  ENTER SELECT  ESC QUIT";
constexpr char kMainMenuFooterShort[] = "ARROWS MOVE  ENTER SELECT  ESC QUIT";

constexpr char kOptionsTitle[] = "OPTIONS";
constexpr char kOptionsSubtitle[] = "CONTROLS + AUDIO";
constexpr char kOptionsWaitingValue[] = "PRESS INPUT...";
constexpr char kOptionsToggleOn[] = "ON";
constexpr char kOptionsToggleOff[] = "OFF";
constexpr char kOptionsFooterWaiting[] = "PRESS KEY/BUTTON OR MOVE STICK  ESC CANCEL";
constexpr char kOptionsFooterWaitingShort[] = "INPUT OR STICK  ESC CANCEL";
constexpr char kOptionsFooterDefault[] = "UP/DOWN MOVE  LEFT/RIGHT ADJUST  ENTER SELECT  ESC BACK";
constexpr char kOptionsFooterDefaultShort[] = "ARROWS MOVE  L/R ADJUST  ENTER SELECT  ESC BACK";

constexpr char kPauseTitle[] = "PAUSED";
constexpr char kPauseResume[] = "RESUME";
constexpr char kPauseQuit[] = "QUIT";
constexpr char kPauseExitDefault[] = "EXIT MATCH";
constexpr char kPauseFooterBase[] = "UP/DOWN MOVE  ENTER SELECT  ESC RESUME";
constexpr char kPauseFooterBaseShort[] = "ARROWS MOVE  ENTER SELECT  ESC RESUME";
constexpr char kPauseForfeitPrompt[] = "FORFEIT THIS MATCH?";
constexpr char kPauseConfirmFooter[] = "LEFT/RIGHT CHOOSE  ENTER CONFIRM  ESC CANCEL";
constexpr char kPauseConfirmFooterShort[] = "L/R CHOOSE  ENTER OK  ESC CANCEL";

constexpr char kQuickMenuTitle[] = "MATCH SETUP";
constexpr char kQuickMenuFooter[] = "UP/DOWN MOVE  LEFT/RIGHT ADJUST  ENTER SELECT  ESC BACK";
constexpr char kQuickMenuFooterShort[] = "ARROWS MOVE/ADJUST  ENTER  ESC BACK";
constexpr char kQuickMenuRowP1Mode[] = "P1 MODE";
constexpr char kQuickMenuRowP2Mode[] = "P2 MODE";
constexpr char kQuickMenuRowP1Style[] = "P1 TUNING";
constexpr char kQuickMenuRowP2Style[] = "P2 TUNING";
constexpr char kQuickMenuRowStartMatch[] = "START MATCH";
constexpr char kQuickMenuOptionHuman[] = "HUMAN";
constexpr char kQuickMenuOptionAi[] = "AI";
constexpr char kQuickMenuOptionStart[] = "START";

constexpr char kStoryMenuTitle[] = "STORY MODE";
constexpr char kStoryMenuSubtitle[] = "VERTICAL SLICE IN PROGRESS";
constexpr char kStoryMenuNoSaveSuffix[] = " (NO SAVE)";
constexpr char kStoryMenuFooter[] = "UP/DOWN MOVE  ENTER SELECT  ESC BACK";
constexpr char kStoryMenuFooterShort[] = "ARROWS MOVE  ENTER SELECT  ESC BACK";
constexpr char kStoryMenuOverwritePrompt[] = "OVERWRITE EXISTING CAREER?";
constexpr char kStoryMenuOverwriteHelp[] = "ENTER CONFIRM, ESC CANCEL";
constexpr char kStoryMenuOverwriteHelpShort[] = "ENTER CONFIRM  ESC CANCEL";
constexpr char kStoryMenuCancel[] = "CANCEL";
constexpr char kStoryMenuOverwrite[] = "OVERWRITE";

constexpr char kStoryHubTitle[] = "STORY HUB";
constexpr char kStoryHubPower[] = "POW";
constexpr char kStoryHubTechnical[] = "TEC";
constexpr char kStoryHubSpin[] = "SPN";
constexpr char kStoryHubFooter[] = "UP/DOWN MOVE  ENTER SELECT  ESC BACK";
constexpr char kStoryHubFooterShort[] = "ARROWS MOVE  ENTER SELECT  ESC BACK";

constexpr char kStoryDialogueFooterWriting[] = "ENTER/SPACE SKIP  HOLD FAST";
constexpr char kStoryDialogueFooterWritingShort[] = "ENTER SKIP  HOLD FAST";
constexpr char kStoryDialogueFooterContinue[] = "PRESS ENTER";
constexpr char kStoryDialogueFooterContinueShort[] = "ENTER CONTINUE";
constexpr char kStoryDialogueFooterChoice[] = "LEFT/RIGHT CHOOSE  ENTER CONFIRM";
constexpr char kStoryDialogueFooterChoiceShort[] = "L/R CHOOSE  ENTER CONFIRM";

constexpr char kNoLabel[] = "NO";
constexpr char kYesLabel[] = "YES";

}  // namespace

namespace whacker::app::ui_text {

const char* unknown_label() {
    return kUnknownLabel;
}

const char* selected_marker() {
    return kSelectedMarker;
}

const char* unselected_marker() {
    return kUnselectedMarker;
}

const char* main_menu_title() {
    return kMainMenuTitle;
}

const char* main_menu_subtitle() {
    return kMainMenuSubtitle;
}

const char* main_menu_footer() {
    return kMainMenuFooter;
}

const char* main_menu_footer_short() {
    return kMainMenuFooterShort;
}

const char* options_title() {
    return kOptionsTitle;
}

const char* options_subtitle() {
    return kOptionsSubtitle;
}

const char* options_waiting_value() {
    return kOptionsWaitingValue;
}

const char* options_toggle_on() {
    return kOptionsToggleOn;
}

const char* options_toggle_off() {
    return kOptionsToggleOff;
}

const char* options_footer_waiting_for_key() {
    return kOptionsFooterWaiting;
}

const char* options_footer_waiting_for_key_short() {
    return kOptionsFooterWaitingShort;
}

const char* options_footer_default() {
    return kOptionsFooterDefault;
}

const char* options_footer_default_short() {
    return kOptionsFooterDefaultShort;
}

const char* pause_title() {
    return kPauseTitle;
}

const char* pause_resume_label() {
    return kPauseResume;
}

const char* pause_quit_label() {
    return kPauseQuit;
}

const char* pause_default_exit_label() {
    return kPauseExitDefault;
}

std::string pause_footer(const std::string_view blocked_reason) {
    std::string footer = kPauseFooterBase;
    if (!blocked_reason.empty()) {
        footer += "  ";
        footer += blocked_reason;
    }
    return footer;
}

std::string pause_footer_short(const std::string_view blocked_reason) {
    std::string footer = kPauseFooterBaseShort;
    if (!blocked_reason.empty()) {
        footer += "  ";
        footer += blocked_reason;
    }
    return footer;
}

const char* pause_forfeit_prompt() {
    return kPauseForfeitPrompt;
}

const char* pause_confirm_footer() {
    return kPauseConfirmFooter;
}

const char* pause_confirm_footer_short() {
    return kPauseConfirmFooterShort;
}

const char* quick_menu_title() {
    return kQuickMenuTitle;
}

const char* quick_menu_footer() {
    return kQuickMenuFooter;
}

const char* quick_menu_footer_short() {
    return kQuickMenuFooterShort;
}

const char* quick_menu_row_p1_mode() {
    return kQuickMenuRowP1Mode;
}

const char* quick_menu_row_p2_mode() {
    return kQuickMenuRowP2Mode;
}

const char* quick_menu_row_p1_style() {
    return kQuickMenuRowP1Style;
}

const char* quick_menu_row_p2_style() {
    return kQuickMenuRowP2Style;
}

const char* quick_menu_row_start_match() {
    return kQuickMenuRowStartMatch;
}

const char* quick_menu_option_human() {
    return kQuickMenuOptionHuman;
}

const char* quick_menu_option_ai() {
    return kQuickMenuOptionAi;
}

const char* quick_menu_option_start() {
    return kQuickMenuOptionStart;
}

const char* story_menu_title() {
    return kStoryMenuTitle;
}

const char* story_menu_subtitle() {
    return kStoryMenuSubtitle;
}

const char* story_menu_no_save_suffix() {
    return kStoryMenuNoSaveSuffix;
}

const char* story_menu_footer() {
    return kStoryMenuFooter;
}

const char* story_menu_footer_short() {
    return kStoryMenuFooterShort;
}

const char* story_menu_overwrite_prompt() {
    return kStoryMenuOverwritePrompt;
}

const char* story_menu_overwrite_help() {
    return kStoryMenuOverwriteHelp;
}

const char* story_menu_overwrite_help_short() {
    return kStoryMenuOverwriteHelpShort;
}

const char* story_menu_cancel_label() {
    return kStoryMenuCancel;
}

const char* story_menu_overwrite_label() {
    return kStoryMenuOverwrite;
}

const char* story_hub_title() {
    return kStoryHubTitle;
}

std::string story_hub_player_line(const std::string& player_name, const int week, const int rating) {
    return "PLAYER " + player_name + "  WEEK " + std::to_string(week) + "  RATING " + std::to_string(rating);
}

std::string story_hub_next_match_line(const bool match_done, const bool has_next_week, const int training_count) {
    if (match_done && !has_next_week) {
        return "END OF AUTHORED CONTENT  TRAINING " + std::to_string(training_count);
    }
    return "NEXT MATCH " + std::string(match_done ? "DONE" : "PENDING") + "  TRAINING " +
        std::to_string(training_count);
}

std::string story_hub_record_line(const int wins, const int losses) {
    return "RECORD " + std::to_string(wins) + "-" + std::to_string(losses);
}

const char* story_hub_power_label() {
    return kStoryHubPower;
}

const char* story_hub_technical_label() {
    return kStoryHubTechnical;
}

const char* story_hub_spin_label() {
    return kStoryHubSpin;
}

const char* story_hub_footer() {
    return kStoryHubFooter;
}

const char* story_hub_footer_short() {
    return kStoryHubFooterShort;
}

const char* story_dialogue_footer_writing() {
    return kStoryDialogueFooterWriting;
}

const char* story_dialogue_footer_writing_short() {
    return kStoryDialogueFooterWritingShort;
}

const char* story_dialogue_footer_continue() {
    return kStoryDialogueFooterContinue;
}

const char* story_dialogue_footer_continue_short() {
    return kStoryDialogueFooterContinueShort;
}

const char* story_dialogue_footer_choice() {
    return kStoryDialogueFooterChoice;
}

const char* story_dialogue_footer_choice_short() {
    return kStoryDialogueFooterChoiceShort;
}

const char* no_label() {
    return kNoLabel;
}

const char* yes_label() {
    return kYesLabel;
}

std::string selected_option_label(const char* label, const bool selected) {
    if (!selected) {
        return label != nullptr ? std::string(label) : std::string {};
    }
    return std::string("> ") + (label != nullptr ? label : "");
}

}  // namespace whacker::app::ui_text
