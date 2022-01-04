#pragma once

#include <glm/glm.hpp>

#include "../../../rendering/view/frustum.hpp"

namespace Platform::Game::Components {
    struct CameraParameters {
        glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;
        glm::vec3 location;
        Rendering::View::Frustum<float> frustum;
    };
}