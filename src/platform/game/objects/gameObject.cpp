#include "gameObject.hpp"

#include <algorithm>
#include <boost/range/adaptor/map.hpp>

#include "../components/gameComponent.hpp"

namespace Platform::Game::Objects {
    GameObject::GameObject() {
        static unsigned int next_id = 1;
        this->id = next_id++;
    }

    void GameObject::render(Components::CameraParameters& cameraParameters) {
        for (boost::shared_ptr<Components::GameComponent> const &component : this->components | boost::adaptors::map_values) {
            component->onRender(cameraParameters);
        }
    }

    void GameObject::onTick(float dt) {
        for (boost::shared_ptr<Components::GameComponent> const &component : this->components | boost::adaptors::map_values) {
            component->onTick(dt);
        }
    }

    bool GameObject::onInputEvent(Input::InputEvent& inputEvent) {
//        return std::any_of(
//                this->components.begin(),
//                this->components.end(),
//                [&](auto const &component) -> bool { return component.second->onInputEvent(inputEvent); }
//        );
        for (boost::shared_ptr<Components::GameComponent> const &component : this->components | boost::adaptors::map_values) {
            if (component->onInputEvent(inputEvent)) {
                return true;
            }
        }
        return false;
    }
}