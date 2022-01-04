#pragma once

// naga
#include "../utils/enum.hpp"

namespace Input {
    DEFINE_ENUM_WITH_STRING_CONVERSIONS(InputDeviceType, (MOUSE)(KEYBOARD)(GAMEPAD))
}
