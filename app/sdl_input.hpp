#pragma once

#include "action_input.hpp"

namespace whacker::app {

class SdlKeyboardInput {
public:
    void sample();

    const ActionInputFrame& frame() const;

private:
    KeyboardPhysicalState previous_ {};
    KeyboardPhysicalState current_ {};
    ActionInputFrame frame_ {};
};

}  // namespace whacker::app
