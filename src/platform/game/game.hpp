#pragma once

#ifndef QUAKE_GAME_HPP
#define QUAKE_GAME_HPP

#include "../../input/inputEvent.hpp"
#include "../../core/windowEvent.hpp"

namespace Platform::Game {
    struct Game {
        virtual void onRunStart() = 0;
        virtual void onRunEnd() = 0;
        virtual void onTickStart(float) = 0;
        virtual void onTickEnd(float) = 0;
        virtual void onRenderStart() = 0;
        virtual void onRenderEnd() = 0;
        virtual bool onInputEvent(Input::InputEvent& input_event) = 0;
        virtual void onWindowEvent(Core::WindowEvent& window_event) = 0;
    };
}

#endif //QUAKE_GAME_HPP
