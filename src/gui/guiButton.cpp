//naga
#include "guiButton.hpp"
#include "../physics/collision.hpp"
#include "../input/inputEvent.hpp"

namespace GUI {
    bool GUIButton::onInputEventBegin(Input::InputEvent& input_event) {
        if (input_event.type.device == Input::InputDeviceType::MOUSE) {
            auto isContained = Physics::contains(getBounds(), input_event.mouse.location());

            switch (input_event.type.action) {
                case Input::InputActionType::PRESS:
                    if (!isContained) {
                        break;
                    }
                    state = State::PRESSED;
                    if (onStateChanged) {
                        auto thisPtr = shared_from_this();
                        onStateChanged(thisPtr);
                    }
                    return true;
                    break;
                case Input::InputActionType::RELEASE:
                    if (!isContained || state != State::PRESSED) {
                        break;
                    }
                    state = State::HOVER;
                    if (onStateChanged) {
                        auto thisPtr = shared_from_this();
                        onStateChanged(thisPtr);
                    }
                    if (onPress) {
                        auto thisPtr = shared_from_this();
                        onPress(thisPtr);
                    }
                    return true;
                    break;
                case Input::InputActionType::MOVE:
                    switch (state) {
                        case State::IDLE:
                            if (!isContained) {
                                break;
                            }
                            state = State::HOVER;
                            if (onStateChanged) {
                                auto thisPtr = shared_from_this();
                                onStateChanged(thisPtr);
                            }

                            return true;
                            break;
                        case State::HOVER:
                        case State::PRESSED:
                            if (!isContained) {
                                break;
                            }
                            state = State::IDLE;
                            if (onStateChanged) {
                                auto thisPtr = shared_from_this();
                                onStateChanged(thisPtr);
                            }
                            return true;
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
        }
        return false;
    }
}
