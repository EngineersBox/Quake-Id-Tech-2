#pragma once

#include "../../core/windowEvent.hpp"

#include <boost/enable_shared_from_this.hpp>

namespace Platform {
    struct InputEvent;
    struct GUILayout;

    struct State : public boost::enable_shared_from_this<State> {
        State();

        virtual void tick(float dt);
        virtual void render();
        virtual bool on_input_event(InputEvent& input_event);
        virtual void on_window_event(Core::WindowEvent& window_event);
        virtual void on_active() { }
        virtual void on_passive() { }
        virtual void on_enter();
        virtual void on_exit() { }
        virtual void on_stop_tick() { }
        virtual void on_start_tick() { }
        virtual void on_stop_render() { }
        virtual void on_start_render() { }
        virtual void on_stop_input() { }
        virtual void on_start_input() { }
        virtual void on_tick(float dt) { }

        const boost::shared_ptr<GUILayout>& get_layout() const { return layout; }

    private:
        boost::shared_ptr<GUILayout> layout;
    };
}
