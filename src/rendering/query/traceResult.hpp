#pragma once

#ifndef QUAKE_TRACERESULT_HPP
#define QUAKE_TRACERESULT_HPP

#include <glm/glm.hpp>

namespace Rendering::Query {
    struct TraceResult {
        bool didHit = false;
        glm::vec3 location;
        glm::vec3 normal;
        void* optional = nullptr;
    };
}

#endif //QUAKE_TRACERESULT_HPP
