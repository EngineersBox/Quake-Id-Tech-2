#pragma once

#include <boost/function.hpp>

#include "guiNode.hpp"

namespace GUI {
    struct GUIButton : GUINode {
        enum class State {
            IDLE,
            HOVER,
            PRESSED
        };

        virtual bool onInputEventBegin(Input::InputEvent& input_event) override;

        State get_state() const { return this->state; }

        boost::function<void(boost::shared_ptr<GUINode>&)> onStateChanged;
        boost::function<void(boost::shared_ptr<GUINode>&)> onPress;

    private:
        State state = State::IDLE;
    };
}
