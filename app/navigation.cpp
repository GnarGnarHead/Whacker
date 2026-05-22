#include "navigation.hpp"

namespace whacker::app {

void push_screen(NavigationState& navigation, const Screen screen) {
    navigation.stack.push_back(navigation.current);
    navigation.current = screen;
}

bool pop_screen(NavigationState& navigation) {
    if (navigation.stack.empty()) {
        return false;
    }
    navigation.current = navigation.stack.back();
    navigation.stack.pop_back();
    return true;
}

void replace_screen(NavigationState& navigation, const Screen screen) {
    if (!navigation.stack.empty() && navigation.stack.back() == screen) {
        navigation.current = navigation.stack.back();
        navigation.stack.pop_back();
        return;
    }
    navigation.current = screen;
}

void reset_to_root(NavigationState& navigation, const Screen screen) {
    navigation.current = screen;
    navigation.stack.clear();
}

bool has_previous_screen(const NavigationState& navigation) {
    return !navigation.stack.empty();
}

Screen previous_screen_or(const NavigationState& navigation, const Screen fallback) {
    return navigation.stack.empty() ? fallback : navigation.stack.back();
}

AppState navigation_app_state(const NavigationState& navigation) {
    return app_state_for_screen(navigation.current);
}

}  // namespace whacker::app
