#pragma once

#ifndef QUAKE_WINDOWEVENT_HPP
#define QUAKE_WINDOWEVENT_HPP

#include "../scene/structure/rectangle.hpp"

namespace Core {
    enum class WindowEventType : unsigned char {
        RESIZE,
        MOVE
    };

    struct WindowEvent {
        typedef Scene::Structure::Rectangle<unsigned short> RectangleType;

        WindowEventType type;
        RectangleType rectangle;
    };
}

#endif //QUAKE_WINDOWEVENT_HPP
