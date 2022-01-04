#pragma once

#include "inputDevice.hpp"
#include "inputEventFlags.hpp"
#include "../utils/enum.hpp"
#include "keyboard/keyboard.hpp"

namespace Input {
    DEFINE_ENUM_WITH_STRING_CONVERSIONS(InputActionType, (PRESS)(RELEASE)(REPEAT)(MOVE)(CHARACTER)(SCROLL))

    // TODO: we would like some key aliases for look-up purposes

    struct InputEventType {
        InputDeviceType device;
        InputActionType action;
        union {
            Input::Keyboard::Key key;
            int button;
            int axis;
            int direction;
            wchar_t character;
        };
        unsigned char flags;

        friend std::ostream& operator<<(std::ostream& ostream, const InputEventType& input_event_type);
        friend std::istream& operator>>(std::istream& istream, const InputEventType& input_event_type);
    };
}