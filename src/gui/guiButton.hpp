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

        State getState() const { return this->state; }

        boost::function<void(boost::shared_ptr<GUINode>&)> onStateChanged;
        boost::function<void(boost::shared_ptr<GUINode>&)> onPress;

    private:
        bool handlePress(bool isContained);
        bool handleRelease(bool isContained);
        bool handleMove(bool isContained);

        State state = State::IDLE;
    };
}
