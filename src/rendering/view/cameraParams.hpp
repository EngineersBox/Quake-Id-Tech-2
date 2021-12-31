#pragma once

#include <glm/glm.hpp>

#include "frustum.hpp"

namespace Rendering::View {
    struct CameraParameters {
        glm::mat4 view_matrix;
        glm::mat4 projection_matrix;
        glm::vec3 location;
        Frustum<float> frustum;
    };
}