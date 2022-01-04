#pragma once

#ifndef QUAKE_CAMERACOMPONENT_HPP
#define QUAKE_CAMERACOMPONENT_HPP

#include "../../../scene/structure/line.hpp"
#include "../../../rendering/view/frustum.hpp"
#include "cameraParams.hpp"
#include "../../../device/gpu/gpuDefs.hpp"

namespace Platform::Game::Components {
    struct CameraComponent {
        static const char* componentName;

        enum class ProjectionType {
            ORTHOGRAPHIC,
            PERSPECTIVE,
        };

        float near = 0.25f;
        float far = 8192.0f;
        float fov = 90.0f;
        float roll = 0.0f;
        ProjectionType projectionType = ProjectionType::PERSPECTIVE;

        Scene::Structure::Line3<float> getRay(const Device::GPU::GpuViewportType& viewport, const glm::vec2& sreenLocation) const;
        CameraParameters getParameters(const Device::GPU::GpuViewportType& viewport) const;

        CameraComponent();
        ~CameraComponent();
    private:
        glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;
        Rendering::View::Frustum<float> frustum;
    };
}

#endif //QUAKE_CAMERACOMPONENT_HPP
