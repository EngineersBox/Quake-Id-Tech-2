#pragma once

#include <string>
#include <glm/glm.hpp>

#include "inputDevice.hpp"
#include "inputEventType.hpp"

namespace Input {
    struct InputEvent {
        struct Mouse {
            size_t id;
            float x, y, dx, dy;

            inline glm::vec2 location() const { return glm::vec2(x, y); }
            inline glm::vec2 locationDelta() const { return glm::vec2(dx, dy); }
        };

        struct Gamepad {
            unsigned char index;
            float value;
            float delta;
        };

        InputEvent() = default;
        InputEvent(const std::string& string);

        size_t id = 0;
        InputEventType type;

        union {
            Mouse mouse;
            Gamepad gamepad;
        };

    private:
        friend std::ostream& operator<<(std::ostream& ostream, const InputEvent& input_event);
    };
}