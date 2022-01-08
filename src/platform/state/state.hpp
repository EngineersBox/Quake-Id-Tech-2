#pragma once

#include "../../input/inputEvent.hpp"
#include "../../core/windowEvent.hpp"

#include <boost/enable_shared_from_this.hpp>

namespace Platform::State {
    struct GUILayout;

    struct State : public boost::enable_shared_from_this<State> {
        State();

        virtual void tick(float dt);
        virtual void render();
        virtual bool onInputEvent(Input::InputEvent& input_event);
        virtual void onWindowEvent(Core::WindowEvent& window_event);
        virtual void onActive() { }
        virtual void onPassive() { }
        virtual void onEnter();
        virtual void onExit() { }
        virtual void onStopTick() { }
        virtual void onStartTick() { }
        virtual void onStopRender() { }
        virtual void onStartRender() { }
        virtual void onStopInput() { }
        virtual void onStartInput() { }
        virtual void onTick(float dt) { }

        const boost::shared_ptr<GUILayout>& getLayout() const { return this->layout; }

    private:
        boost::shared_ptr<GUILayout> layout;
    };
}
