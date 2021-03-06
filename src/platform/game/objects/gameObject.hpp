#pragma once

#ifndef QUAKE_GAMEOBJECT_HPP
#define QUAKE_GAMEOBJECT_HPP

#include <vector>
#include <map>
#include <set>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <type_traits>

#include "../models/pose.hpp"
#include "../components/cameraParams.hpp"
#include "../components/gameComponent.hpp"

namespace Scenes { struct Scene; }
namespace Input { struct InputEvent; }

namespace Platform::Game::Objects {
    struct GameObject : boost::enable_shared_from_this<GameObject> {
        GameObject();

        Models::Pose3<float> pose;

        virtual void onCreate() { }
        virtual void onDestroy() { }
        virtual bool onInputEvent(Input::InputEvent& input_event);
        virtual void onTick(float dt);
        virtual void render(Components::CameraParameters& camera_parameters);

        template<typename T> requires Components::IsGameComponent<T>
        boost::shared_ptr<T> addComponent(const char* name, T& component) {
            boost::shared_ptr<T> _component = boost::make_shared<T>(component);
            _component->owner = shared_from_this();
            if (this->components.find(name) != this->components.end()) {
                throw std::runtime_error("Component with name " + std::string(name) + " already exists");
            }

            this->components.emplace(name, _component);
            _component->onCreate();
            return _component;
        }

        template<typename T> requires Components::IsGameComponent<T>
        boost::shared_ptr<T> getComponent(const char* name) const {
            auto entry = this->components.find(name);
            if (entry == this->components.end()) {
                throw std::runtime_error("Component with name " + std::string(name) + " does not exist");
            }
            return boost::static_pointer_cast<T, Components::GameComponent>(entry->second);
        }

        const boost::shared_ptr<Scenes::Scene>& getScene() const { return this->scene; }
        size_t getId() const { return this->id; }

        boost::shared_ptr<Scenes::Scene> scene;
    private:
        friend struct Scene;

        size_t id;
        std::map<std::string, boost::shared_ptr<Components::GameComponent>> components;
    };
}

#endif //QUAKE_GAMEOBJECT_HPP
