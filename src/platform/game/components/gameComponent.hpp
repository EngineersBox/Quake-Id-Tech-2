#pragma once

#ifndef QUAKE_GAMECOMPONENT_HPP
#define QUAKE_GAMECOMPONENT_HPP

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "gameComponentDefs.hpp"
#include "cameraParams.hpp"

namespace Input {
    struct InputEvent;
}

namespace Platform::Game {
    namespace Objects {
        struct GameObject;
    }

    namespace Components {
        struct GameComponent : boost::enable_shared_from_this<GameComponent> {
            [[nodiscard]] const boost::shared_ptr<Objects::GameObject>& getOwner() const { return this->owner; }

            virtual std::string getComponentName() const { return ""; }

            virtual void onCreate() { }
            virtual void onDestroy() { }
            virtual void onTick(float dt) { }
            virtual void onRender(Components::CameraParameters& cameraParameters) { }
            virtual bool onInputEvent(Input::InputEvent& inputEvent) { return false; }

            virtual ~GameComponent() = default;

            boost::shared_ptr<Objects::GameObject> owner;
        protected:
            GameComponent() = default;

        private:
            friend struct Objects::GameObject;
        };

        template<typename T>
        concept IsGameComponent = std::derived_from<T, GameComponent>;
    }
}

#endif //QUAKE_GAMECOMPONENT_HPP
