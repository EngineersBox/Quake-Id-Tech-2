#pragma once

#ifndef QUAKE_MONITOR_HPP
#define QUAKE_MONITOR_HPP

#include <glm/glm.hpp>

namespace Device {
    struct Monitor {
        const char* name = nullptr;
        glm::ivec2 physicalSize;
        glm::ivec2 position;
    };
}

#endif //QUAKE_MONITOR_HPP
