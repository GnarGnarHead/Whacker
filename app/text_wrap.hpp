#pragma once

#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

namespace whacker::app {

std::vector<std::string> wrap_text_to_char_lines(
    const std::string& raw,
    int max_chars_per_line,
    int max_lines);

std::string fit_text_to_single_line(
    const std::string& raw,
    int max_chars_per_line);

std::string fit_text_to_single_line_ellipsis(
    const std::string& raw,
    int max_chars_per_line);

std::string choose_best_fitting_variant(
    std::initializer_list<std::string_view> variants,
    int max_chars_per_line);

}  // namespace whacker::app
