//naga
#include "guiButton.hpp"
#include "../physics/collision.hpp"
#include "../input/inputEvent.hpp"

namespace GUI {
    bool GUIButton::handlePress(bool isContained) {
        if (!isContained) return false;
        this->state = State::PRESSED;
        if (onStateChanged) {
            auto thisPtr = shared_from_this();
            onStateChanged(thisPtr);
        }
        return true;
    }

    bool GUIButton::handleRelease(bool isContained) {
        if (!isContained || this->state != State::PRESSED) return false;
        this->state = State::HOVER;
        if (onStateChanged) {
            auto thisPtr = shared_from_this();
            onStateChanged(thisPtr);
        }
        if (onPress) {
            auto thisPtr = shared_from_this();
            onPress(thisPtr);
        }
        return true;
    }

    bool GUIButton::handleMove(bool isContained) {
        switch (state) {
            case State::IDLE:
                if (!isContained) break;
                state = State::HOVER;
                if (onStateChanged) {
                    auto thisPtr = shared_from_this();
                    onStateChanged(thisPtr);
                }
                return true;
            case State::HOVER:
            case State::PRESSED:
                if (!isContained) break;
                state = State::IDLE;
                if (onStateChanged) {
                    auto thisPtr = shared_from_this();
                    onStateChanged(thisPtr);
                }
                return true;
        }
        return false;
    }

    bool GUIButton::onInputEventBegin(Input::InputEvent& input_event) {
        if (input_event.type.device != Input::InputDeviceType::MOUSE) {
            return false;
        }
        bool isContained = Physics::contains(getBounds(), input_event.mouse.location());

        switch (input_event.type.action) {
            case Input::InputActionType::PRESS:
                return handlePress(isContained);
            case Input::InputActionType::RELEASE:
                return handleRelease(isContained);
            case Input::InputActionType::MOVE:
                return handleMove(isContained);
            default:
                break;
        }
    }
}
