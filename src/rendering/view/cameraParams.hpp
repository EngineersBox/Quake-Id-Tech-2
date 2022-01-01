#pragma once

#include <glm/glm.hpp>

#include "frustum.hpp"

namespace Rendering::View {
    struct CameraParameters {
        glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;
        glm::vec3 location;
        Frustum<float> frustum;
    };
}