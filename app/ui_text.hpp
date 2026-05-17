#pragma once

#include <string>
#include <string_view>

namespace whacker::app::ui_text {

const char* unknown_label();
const char* selected_marker();
const char* unselected_marker();

const char* main_menu_title();
const char* main_menu_subtitle();
const char* main_menu_footer();
const char* main_menu_footer_short();

const char* options_title();
const char* options_subtitle();
const char* options_waiting_value();
const char* options_toggle_on();
const char* options_toggle_off();
const char* options_footer_waiting_for_key();
const char* options_footer_waiting_for_key_short();
const char* options_footer_default();
const char* options_footer_default_short();

const char* pause_title();
const char* pause_resume_label();
const char* pause_quit_label();
const char* pause_default_exit_label();
std::string pause_footer(std::string_view blocked_reason);
std::string pause_footer_short(std::string_view blocked_reason);
const char* pause_forfeit_prompt();
const char* pause_confirm_footer();
const char* pause_confirm_footer_short();

const char* quick_menu_title();
const char* quick_menu_footer();
const char* quick_menu_footer_short();
const char* quick_menu_row_p1_mode();
const char* quick_menu_row_p2_mode();
const char* quick_menu_row_p1_style();
const char* quick_menu_row_p2_style();
const char* quick_menu_row_start_match();
const char* quick_menu_option_human();
const char* quick_menu_option_ai();
const char* quick_menu_option_start();

const char* story_menu_title();
const char* story_menu_subtitle();
const char* story_menu_no_save_suffix();
const char* story_menu_footer();
const char* story_menu_footer_short();
const char* story_menu_overwrite_prompt();
const char* story_menu_overwrite_help();
const char* story_menu_overwrite_help_short();
const char* story_menu_cancel_label();
const char* story_menu_overwrite_label();

const char* story_hub_title();
std::string story_hub_player_line(const std::string& player_name, int week, int rating);
std::string story_hub_next_match_line(bool match_done, bool has_next_week, int training_count);
std::string story_hub_record_line(int wins, int losses);
const char* story_hub_power_label();
const char* story_hub_technical_label();
const char* story_hub_spin_label();
const char* story_hub_footer();
const char* story_hub_footer_short();

const char* story_dialogue_footer_writing();
const char* story_dialogue_footer_writing_short();
const char* story_dialogue_footer_continue();
const char* story_dialogue_footer_continue_short();
const char* story_dialogue_footer_choice();
const char* story_dialogue_footer_choice_short();

const char* no_label();
const char* yes_label();
std::string selected_option_label(const char* label, bool selected);

}  // namespace whacker::app::ui_text
