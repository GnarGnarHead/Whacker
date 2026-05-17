#include "text_wrap.hpp"

#include <algorithm>
#include <cctype>
#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

namespace whacker::app {

namespace {

std::string trim_left_copy(const std::string& value) {
    std::size_t i = 0;
    while (i < value.size() && std::isspace(static_cast<unsigned char>(value[i]))) {
        ++i;
    }
    return value.substr(i);
}

std::string trim_right_copy(const std::string& value) {
    if (value.empty()) {
        return value;
    }
    std::size_t i = value.size();
    while (i > 0 && std::isspace(static_cast<unsigned char>(value[i - 1]))) {
        --i;
    }
    return value.substr(0, i);
}

std::string trim_copy(const std::string_view value) {
    return trim_left_copy(trim_right_copy(std::string(value)));
}

}  // namespace

std::vector<std::string> wrap_text_to_char_lines(
    const std::string& raw,
    const int max_chars_per_line,
    const int max_lines) {
    std::vector<std::string> lines;
    if (max_chars_per_line <= 0 || max_lines <= 0) {
        return lines;
    }

    std::string remaining = trim_left_copy(trim_right_copy(raw));
    while (!remaining.empty() && static_cast<int>(lines.size()) < max_lines) {
        if (static_cast<int>(remaining.size()) <= max_chars_per_line) {
            lines.push_back(remaining);
            remaining.clear();
            break;
        }

        std::size_t cut = static_cast<std::size_t>(max_chars_per_line);
        const std::size_t space = remaining.rfind(' ', cut);
        if (space != std::string::npos && space > 0) {
            cut = space;
        }

        std::string line = trim_right_copy(remaining.substr(0, cut));
        if (line.empty()) {
            line = remaining.substr(0, static_cast<std::size_t>(max_chars_per_line));
            cut = static_cast<std::size_t>(max_chars_per_line);
        }

        lines.push_back(line);
        remaining = trim_left_copy(remaining.substr(cut));
    }

    if (!remaining.empty() && !lines.empty()) {
        std::string& last = lines.back();
        if (max_chars_per_line > 3) {
            if (static_cast<int>(last.size()) > (max_chars_per_line - 3)) {
                last = last.substr(0, static_cast<std::size_t>(max_chars_per_line - 3));
            }
            last += "...";
        } else if (static_cast<int>(last.size()) > max_chars_per_line) {
            last = last.substr(0, static_cast<std::size_t>(max_chars_per_line));
        }
    }

    return lines;
}

std::string fit_text_to_single_line(const std::string& raw, const int max_chars_per_line) {
    return fit_text_to_single_line_ellipsis(raw, max_chars_per_line);
}

std::string fit_text_to_single_line_ellipsis(const std::string& raw, const int max_chars_per_line) {
    const std::vector<std::string> lines = wrap_text_to_char_lines(raw, max_chars_per_line, 1);
    if (lines.empty()) {
        return std::string {};
    }
    return lines.front();
}

std::string choose_best_fitting_variant(
    const std::initializer_list<std::string_view> variants,
    const int max_chars_per_line) {
    if (max_chars_per_line <= 0) {
        return std::string {};
    }

    std::string fallback;
    for (const std::string_view variant : variants) {
        const std::string trimmed = trim_copy(variant);
        if (trimmed.empty()) {
            continue;
        }
        fallback = trimmed;
        if (static_cast<int>(trimmed.size()) <= max_chars_per_line) {
            return trimmed;
        }
    }

    if (fallback.empty()) {
        return std::string {};
    }
    return fit_text_to_single_line_ellipsis(fallback, max_chars_per_line);
}

}  // namespace whacker::app
