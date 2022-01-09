//naga
#include "guiButton.hpp"
#include "../physics/collision.hpp"
#include "../input/inputEvent.hpp"

namespace GUI {
    bool GUIButton::onInputEventBegin(Input::InputEvent& input_event) {
        if (input_event.type.device == Input::InputDeviceType::MOUSE) {
            auto is_contained = contains(getBounds(), input_event.mouse.location());

            switch (input_event.type.action) {
                case Input::InputActionType::PRESS:
                    if (!isContained) {
                        break;
                    }
                    state = State::PRESSED;
                    if (onStateChanged) {
                        onStateChanged(shared_from_this());
                    }
                    return true;
                    break;
                case Input::InputActionType::RELEASE:
                    if (!isContained || state != State::PRESSED) {
                        break;
                    }
                    state = State::HOVER;
                    if (onStateChanged) {
                        onStateChanged(shared_from_this());
                    }
                    if (onPress) {
                        onPress(shared_from_this());
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
                            if (onStateShanged) {
                                onStateShanged(shared_from_this());
                            }

                            return true;
                            break;
                        case State::HOVER:
                        case State::PRESSED:
                            if (!isContained) {
                                break;
                            }
                            state = State::IDLE;
                            if (onStateShanged) {
                                onStateShanged(shared_from_this());
                            }
                            return true;
                            break;
                        default:
                            break;
                    }
                    break;
            }
        }
        return false;
    }
}
