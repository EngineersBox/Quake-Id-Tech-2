#include <sstream>
#include <boost/range/adaptor/reversed.hpp>

#include "stateSystem.hpp"
#include "state.hpp"

namespace Platform::State {
    StateSystem states;

    void StateSystem::tick(float dt) {
        bool didNodesChange = !operations.empty();

        StateType previousTopState;

        if (!operations.empty()) {
            previousTopState = nodes.empty() ? nullptr : nodes.rbegin()->state;
        }

        std::vector<StateType> pushedStates;
        std::vector<std::pair<StateType, StateFlagsType>> poppedStates;

        //process stack operations
        while (!operations.empty()) {
            const Operation& operation = operations.front();
            std::__list_iterator<StateSystem::Node, void *> nodesItr = std::find_if(
                    nodes.begin(),
                    nodes.end(),
                    [&](const Node& node) -> bool {
                        return node.state == operation.state;
                    }
            );

            switch (operation.type) {
                case Operation::Type::POP: {
                    if (nodesItr == nodes.end()) {
                        throw std::runtime_error("state does not exist on stack, cannot pop");
                    }

                    poppedStates.emplace_back(nodesItr->state, nodesItr->flags);
                    nodes.erase(nodesItr);
                } break;
                case Operation::Type::PUSH: {
                    if (nodesItr != nodes.end()) {
                        throw std::runtime_error("state already exists on stack, cannot push");
                    }

                    //add state to stack
                    Node node;
                    node.linkFlags = operation.linkFlags;
                    node.state = operation.state;
                    node.weight = operation.weight;

                    nodes.push_back(node);
                    pushedStates.push_back(operation.state);
                } break;
                case Operation::Type::CHANGE_LINK_FLAGS: {
                    if (nodesItr == nodes.end()) {
                        throw std::runtime_error("state does not exist on stack, cannot change link flags");
                    }
                    nodesItr->linkFlags = operation.linkFlags;
                    nodesItr->flags &= ~STATE_FLAG_CHANGING_LINK_FLAGS;
                } break;
                case Operation::Type::PURGE: {
                    while (!nodes.empty()) {
                        poppedStates.emplace_back(nodes.back().state, nodes.back().flags);
                        nodes.pop_back();
                    }
                } break;
                case Operation::Type::CHANGE_WEIGHT: {
                    if (nodesItr == nodes.end()) {
                        throw std::runtime_error("state does not exist on the stack, cannot change weight");
                    }
                    //change weight of state on stack
                    nodesItr->weight = operation.weight;
                } break;
            }

            operations.pop_front();
        }

        for (boost::shared_ptr<State>& state : pushedStates) {
            state->onEnter();
        }

        if (didNodesChange) {
            //sort nodes by weight
            nodes.sort([&](const Node& lhs, const Node& rhs) -> bool {
                return lhs.weight > rhs.weight;
            });
            if (!nodes.empty() && previousTopState != nodes.rbegin()->state) {
                if (previousTopState != nullptr) previousTopState->onPassive();
                if (!nodes.empty()) nodes.rbegin()->state->onActive();
            }

            //compare previous node flags to current ones, call start/stop flag events to states if flags changed
			StateFlagsType flags = STATE_FLAG_ALL;

            for (Node& node : boost::adaptors::reverse(this->nodes)) {
                const unsigned char previousNodeFlags = node.flags;

                //render
                if ((previousNodeFlags & STATE_FLAG_RENDER) == STATE_FLAG_NONE && (flags & STATE_FLAG_RENDER) == STATE_FLAG_RENDER) {
                    node.state->onStartRender();
                } else if ((previousNodeFlags & STATE_FLAG_RENDER) == STATE_FLAG_RENDER && (flags & STATE_FLAG_RENDER) == STATE_FLAG_NONE) {
                    node.state->onStopRender();
                }

                //input
                if ((previousNodeFlags & STATE_FLAG_INPUT) == STATE_FLAG_NONE && (flags & STATE_FLAG_INPUT) == STATE_FLAG_INPUT) {
                    node.state->onStartInput();
                } else if ((previousNodeFlags & STATE_FLAG_INPUT) == STATE_FLAG_INPUT && (flags & STATE_FLAG_INPUT) == STATE_FLAG_NONE) {
                    node.state->onStopInput();
                }

                //tick
                if ((previousNodeFlags & STATE_FLAG_TICK) == STATE_FLAG_NONE && (flags & STATE_FLAG_TICK) == STATE_FLAG_TICK) {
                    node.state->onStartTick();
                } else if ((previousNodeFlags & STATE_FLAG_TICK) == STATE_FLAG_TICK && (flags & STATE_FLAG_TICK) == STATE_FLAG_NONE) {
                    node.state->onStopTick();
                }

                node.flags = flags;
                flags &= node.linkFlags;
            }
        }

        for (std::pair<boost::shared_ptr<State>, Platform::State::StateFlagsType>& state : poppedStates) {
            if ((state.second & STATE_FLAG_RENDER) == STATE_FLAG_RENDER) state.first->onStopRender();
            if ((state.second & STATE_FLAG_INPUT) == STATE_FLAG_INPUT) state.first->onStopInput();
            if ((state.second & STATE_FLAG_TICK) == STATE_FLAG_TICK) state.first->onStopTick();
            state.first->onExit();
        }

        //tick states
        for (Node& node : boost::adaptors::reverse(this->nodes)) {
            if ((node.flags & STATE_FLAG_TICK) == STATE_FLAG_NONE) break;
            node.state->tick(dt);
        }
    }

    void StateSystem::render() {
        for (auto& node : this->nodes) {
            if ((node.flags & STATE_FLAG_RENDER) == STATE_FLAG_RENDER) node.state->render();
        }
    }

    bool StateSystem::onInputEvent(Input::InputEvent& input_event) {
        for (Node& node : boost::adaptors::reverse(this->nodes)) {
            if ((node.flags & STATE_FLAG_INPUT) != STATE_FLAG_INPUT ||
                (node.flags & STATE_FLAG_POPPING) == STATE_FLAG_POPPING) {
                return false;
            }
            if (node.state->onInputEvent(input_event)) return true;
        }
        return false;
    }

    void StateSystem::onWindowEvent(Core::WindowEvent& window_event) {
        for (Node& node : this->nodes) {
            node.state->onWindowEvent(window_event);
        }
    }

    //push a state onto the stack
    void StateSystem::push(const StateType& state, StateFlagsType link_flags, WeightType weight) {
        if (state == nullptr) {
            throw std::invalid_argument("state cannot be null");
        }

        Operation operation;
        operation.type = Operation::Type::PUSH;
        operation.state = state;
        operation.linkFlags = link_flags;
        operation.weight = weight;

        operations.push_back(operation);
    }

    //pop a specific state off of the stack
    void StateSystem::pop(const StateType& state) {
        if (state == nullptr) {
            throw std::invalid_argument("state cannot be null");
        }

        const std::__list_iterator<Node, void*> nodesItr = std::find_if(
                nodes.begin(),
                nodes.end(),
                [&](const Node& node) -> bool {
                    return node.state == state;
                }
        );

        if (nodesItr == nodes.end()) {
            //state does not exist on the stack
            //check operation queue to see if state is about to be pushed
            const auto operationsItr = std::find_if(
                    operations.begin(),
                    operations.end(),
                    [&](const Operation& operation) -> bool {
                        return operation.state == state && operation.type == Operation::Type::PUSH;
                    }
            );

            if (operationsItr != operations.end()) {
                //state is in operation queue to be pushed, erase that operation (effectively a no-op)
                operations.erase(operationsItr);
                return;
            } else {
                throw std::runtime_error("state is not on the stack nor in queue to be pushed");
            }
        }

        //add popping flag to node flags
        nodesItr->flags |= STATE_FLAG_POPPING;

        Operation operation;
        operation.type = Operation::Type::POP;
        operation.state = state;

        operations.push_back(operation);
    }

    void StateSystem::changeLinkFlags(const StateType& state, StateFlagsType link_flags) {
        if (state == nullptr) {
            throw std::invalid_argument("state cannot be null");
        }

        const std::__list_iterator<Node, void*> nodesItr = std::find_if(
                nodes.begin(),
                nodes.end(),
                [&](const Node& node) -> bool {
                    return node.state == state;
                }
        );

        if (nodesItr == nodes.end()) {
            //state does not exist on the stack
            //check operation queue to see if state is about to be pushed
            const auto operationsItr = std::find_if(
                    operations.begin(),
                    operations.end(),
                    [&](const Operation& operation) -> bool {
                        return operation.state == state && operation.type == Operation::Type::PUSH;
                    }
            );

            if (operationsItr != operations.end()) {
                //state is in operation queue to be pushed, change operations' link flags
                operationsItr->linkFlags = link_flags;
                return;
            } else {
                throw std::runtime_error("state is not on the stack nor in queue to be pushed");
            }
        }

        Operation operation;
        operation.type = Operation::Type::CHANGE_LINK_FLAGS;
        operation.state = state;
        operation.linkFlags = link_flags;

        operations.push_back(operation);

        //add changing link flags flag to node flags
        nodesItr->flags |= STATE_FLAG_CHANGING_LINK_FLAGS;
    }

    void StateSystem::changeWeight(const StateType& state, WeightType weight = 0) {
        if (state == nullptr) {
            throw std::invalid_argument("state cannot be null");
        }

        const std::__list_iterator<Node, void*> nodesItr = std::find_if(
                nodes.begin(),
                nodes.end(),
                [&](const Node& node) -> bool {
                    return node.state == state;
                }
        );

        if (nodesItr == nodes.end()) {
            const auto operationsItr = std::find_if(
                    operations.begin(),
                    operations.end(),
                    [&](const Operation& operation) -> bool {
                        return operation.state == state && (operation.type == Operation::Type::PUSH || operation.type == Operation::Type::CHANGE_WEIGHT);
                    }
            );

            if (operationsItr != operations.end()) {
                //state is in operation queue to be pushed or have it's weight changed, change operations' weight
                operationsItr->weight = weight;
                return;
            } else {
                throw std::runtime_error("state is not on the stack nor in queue to be pushed");
            }
        }

        Operation operation;
        operation.type = Operation::Type::CHANGE_WEIGHT;
        operation.state = state;
        operation.weight = weight;
        operations.push_back(operation);
    }

    void StateSystem::purge() {
        Operation operation;
        operation.type = Operation::Type::PURGE;
        operations.push_back(operation);
    }

    size_t StateSystem::count() const {
        return this->nodes.size();
    }

    StateSystem::StateType StateSystem::at(size_t index) const {
        if (index >= nodes.size()) {
            std::ostringstream oss;
            oss << "no state at index " << index;
            throw std::out_of_range(oss.str());
        }
        std::reverse_iterator<std::__list_const_iterator<Node, void*>> nodesReverseItr = nodes.rbegin();
        while (index-- > 0) {
            ++nodesReverseItr;
        }
        return nodesReverseItr->state;
    }

    bool StateSystem::contains(const StateType& state) const {
        return std::find_if(
                nodes.rbegin(),
                nodes.rend(),
                [&](const Node& node) -> bool { return node.state == state; }
        ) != nodes.rend();
    }

    boost::optional<size_t> StateSystem::indexOf(const StateType& state) const {
        boost::optional<size_t> index;
        const std::reverse_iterator<std::__list_const_iterator<Node, void*>> nodesReverseItr = std::find_if(
                nodes.rbegin(),
                nodes.rend(),
                [&](const Node& node) -> bool {
                    return node.state == state;
                }
        );

        if (nodesReverseItr != nodes.rend()) {
            index = std::distance(nodes.rbegin(), nodesReverseItr);
        }
        return index;
    }

    StateFlagsType StateSystem::getLinkFlags(const StateType& state) const {
        const std::__list_const_iterator<Node, void*> nodesItr = std::find_if(
                nodes.begin(),
                nodes.end(),
                [&](const Node& node) -> bool {
                    return node.state == state;
                }
        );

        if (nodesItr == nodes.end()) {
            throw std::out_of_range("state is not on the stack");
        }

        return nodesItr->linkFlags;
    }

    StateFlagsType StateSystem::getFlags(const StateType& state) const {
        const std::__list_const_iterator<Node, void*> nodesItr = std::find_if(
                nodes.begin(),
                nodes.end(),
                [&](const Node& node) -> bool {
                    return node.state == state;
                }
        );

        if (nodesItr == nodes.end()) {
            throw std::out_of_range("state is not on the stack");
        }
        return nodesItr->flags;
    }

    int StateSystem::getWeight(const StateType& state) const {
        const std::__list_const_iterator<Node, void*> nodesItr = std::find_if(
                nodes.begin(),
                nodes.end(),
                [&](const Node& node) -> bool {
                    return node.state == state;
                }
        );

        if (nodesItr == nodes.end()) {
            throw std::out_of_range("state is not on the stack");
        }
        return nodesItr->weight;
    }

    bool StateSystem::isStateRendering(const StateType& state) const {
        return (getFlags(state) & STATE_FLAG_RENDER) == STATE_FLAG_RENDER;
    }

    bool StateSystem::isStateTicking(const StateType& state) const {
        return (getFlags(state) & STATE_FLAG_TICK) == STATE_FLAG_TICK;
    }

    bool StateSystem::isStateHandlingInput(const StateType& state) const {
        return (getFlags(state) & STATE_FLAG_INPUT) == STATE_FLAG_INPUT;
    }

    bool StateSystem::isStatePopping(const StateType& state) const {
        return (getFlags(state) & STATE_FLAG_POPPING) == STATE_FLAG_POPPING;
    }

    bool StateSystem::isStateChangingLinkFlags(const StateType& state) const {
        return (getFlags(state) & STATE_FLAG_CHANGING_LINK_FLAGS) == STATE_FLAG_CHANGING_LINK_FLAGS;
    }
}