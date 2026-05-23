#pragma once

#include "action_input.hpp"
#include "input_profiles.hpp"

namespace whacker::app {

class SdlInput {
public:
    SdlInput();
    explicit SdlInput(InputProfile profile);
    explicit SdlInput(ActionInputBindings bindings);
    ~SdlInput();

    SdlInput(const SdlInput&) = delete;
    SdlInput& operator=(const SdlInput&) = delete;

    void sample();

    const ActionInputFrame& frame() const;
    ActionInputBindings& bindings();
    const ActionInputBindings& bindings() const;
    int controller_index_for_instance_id(int instance_id) const;

private:
    void refresh_controllers();
    void close_controllers();
    InputPhysicalState sample_physical_state() const;

    std::array<void*, kMaxInputControllers> controllers_ {};
    std::array<int, kMaxInputControllers> controller_instance_ids_ {};
    InputPhysicalState previous_ {};
    InputPhysicalState current_ {};
    ActionInputFrame frame_ {};
    ActionInputBindings bindings_ {};
};

const char* sdl_keyboard_scancode_label(int scancode);
bool sdl_keyboard_scancode_bindable(int scancode);

}  // namespace whacker::app
