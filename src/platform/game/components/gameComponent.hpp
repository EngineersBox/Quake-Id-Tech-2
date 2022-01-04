#pragma once

#ifndef QUAKE_GAMECOMPONENT_HPP
#define QUAKE_GAMECOMPONENT_HPP

#include <boost/shared_ptr.hpp>

#include "gameComponentDefs.hpp"
#include "cameraParams.hpp"

namespace Platform::Game {
    struct GameObject;
    struct InputEvent;

    struct GameComponent {
        const boost::shared_ptr<GameObject>& getOwner() const { return this->owner; }

        virtual std::string getComponentName() const { return ""; }

        virtual void onCreate() { }
        virtual void onDestroy() { }
        virtual void onTick(float dt) { }
        virtual void onRender(Components::CameraParameters& cameraParameters) { }
        virtual bool onInputEvent(InputEvent& inputEvent) { return false; }

        virtual ~GameComponent() = default;

        boost::shared_ptr<GameObject> owner;
    protected:
        GameComponent() = default;

    private:
        friend struct GameObject;

    };
}

#endif //QUAKE_GAMECOMPONENT_HPP
