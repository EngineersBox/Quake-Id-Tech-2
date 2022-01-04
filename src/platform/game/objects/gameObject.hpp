//
// Created by Jack Kilrain on 2/1/22.
//

#ifndef QUAKE_GAMEOBJECT_HPP
#define QUAKE_GAMEOBJECT_HPP

#include <vector>
#include <map>
#include <set>

#include <boost/shared_ptr.hpp>
#include <boost/python.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "pose.hpp"
#include "../components/cameraParams.hpp"

namespace Platform::Game {
    struct GameComponent;
    struct InputEvent;

    namespace Objects {
        struct Scene;

        struct GameObject : boost::enable_shared_from_this<GameObject> {
            GameObject();

            Pose3 pose;

            virtual void onCreate() { }
            virtual void onDestroy() { }
            virtual bool onInputEvent(InputEvent& input_event);
            virtual void onTick(float dt);
            virtual void render(Components::CameraParameters& camera_parameters);

            template<typename T, typename std::enable_if_t<std::is_base_of<GameComponent, T>::value, bool>::type = true>
            boost::shared_ptr<T> addComponent(std::string& name, T& component) {
                boost::shared_ptr<GameComponent> _component(component);
                _component->owner = shared_from_this();
                if (this->components.contains(name)) {
                    throw std::runtime_error("Component with name " + name + " already exists");
                }

                this->components.emplace(name, _component);
                _component->onCreate();
                return _component;
            }

            template<typename T, typename std::enable_if_t<std::is_base_of<GameComponent, T>::value, bool>::type = true>
            boost::shared_ptr<T> getComponent(std::string& name) const {
                auto entry = this->components.find(name);
                if (entry == this->components.end()) {
                    throw std::runtime_error("Component with name " + name + " does not exist");
                }
                return boost::static_pointer_cast<T, GameComponent>(entry->second);
            }

            const boost::shared_ptr<Scene>& getScene() const { return this->scene; }
            size_t getId() const { return this->id; }

        private:
            friend struct Scene;

            size_t id;
            std::map<std::string, boost::shared_ptr<GameComponent>> components;
            boost::shared_ptr<Scene> scene;
        };
    }
}

#endif //QUAKE_GAMEOBJECT_HPP
