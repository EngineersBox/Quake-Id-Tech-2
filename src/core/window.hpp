#pragma once

#ifndef QUAKE_WINDOW_HPP
#define QUAKE_WINDOW_HPP

#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace Core {
    struct Display {
        struct VideoMode {
            int width = 0;
            int height = 0;
            int bitDepth = 0;
            int refreshRate = 0;
        };

        std::string name;
        glm::ivec2 position;
        float ppi = 0.0f;
        std::vector<VideoMode> videoModes;
    };

}

#endif //QUAKE_WINDOW_HPP
