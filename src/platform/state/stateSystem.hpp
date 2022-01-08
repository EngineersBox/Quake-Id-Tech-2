#pragma once

#include "../../input/inputEvent.hpp"
#include "../../core/windowEvent.hpp"

#include <vector>
#include <deque>
#include <list>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>

namespace Platform::State {
    struct State;

    typedef unsigned char StateFlagsType;

    enum : StateFlagsType {
        STATE_FLAG_NONE = (0 << 0),
        STATE_FLAG_RENDER = (1 << 0),
        STATE_FLAG_INPUT = (1 << 1),
        STATE_FLAG_TICK = (1 << 2),
        STATE_FLAG_POPPING = (1 << 3),
        STATE_FLAG_CHANGING_LINK_FLAGS = (1 << 4),
        STATE_FLAG_RENDER_INPUT = (STATE_FLAG_RENDER | STATE_FLAG_INPUT),
        STATE_FLAG_RENDER_TICK = (STATE_FLAG_RENDER | STATE_FLAG_TICK),
        STATE_FLAG_INPUT_TICK = (STATE_FLAG_INPUT | STATE_FLAG_TICK),
        STATE_FLAG_ALL = (STATE_FLAG_RENDER | STATE_FLAG_INPUT | STATE_FLAG_TICK)
    };

    struct StateSystem {
        typedef boost::shared_ptr<State> StateType;
        typedef int WeightType;

        struct Node {
            StateType state;
            StateFlagsType linkFlags = STATE_FLAG_NONE;
            StateFlagsType flags = STATE_FLAG_NONE;
            WeightType weight = 0;
        };

        struct Operation {
            enum class Type : unsigned char {
                PUSH,
                POP,
                CHANGE_LINK_FLAGS,
                CHANGE_WEIGHT,
                PURGE
            };

            Type type = Type::PUSH;
            StateType state;
            StateFlagsType linkFlags = STATE_FLAG_NONE;
            size_t index = 0;
            WeightType weight = 0;
        };

        StateSystem() = default;

        void tick(float dt);
        void render();
        bool onInputEvent(Input::InputEvent& input_event);
        void onWindowEvent(Core::WindowEvent& window_event);

        void push(const StateType& state, StateFlagsType link_flags, WeightType weight = 0);
        void pop(const StateType& state);
        void changeLinkFlags(const StateType& state, StateFlagsType link_flags);
        void changeWeight(const StateType& state, WeightType weight);
        void purge();

        StateFlagsType getFlags(const StateType& state) const;
        StateFlagsType getLinkFlags(const StateType& state) const;
        WeightType getWeight(const StateType& state) const;
        size_t count() const;
        StateType at(size_t node_index) const;
        bool contains(const StateType& state) const;
        boost::optional<size_t> indexOf(const StateType& state) const;
        bool isStateRendering(const StateType& state) const;
        bool isStateTicking(const StateType& state) const;
        bool isStateHandlingInput(const StateType& state) const;
        bool isStatePopping(const StateType& state) const;
        bool isStateChangingLinkFlags(const StateType& state) const;
        
    private:
        StateSystem(const StateSystem&) = delete;
        StateSystem& operator=(const StateSystem&) = delete;

        std::list<Node> nodes;
        std::deque<Operation> operations;
    };

    extern StateSystem states;
}