#include "cameraComponent.hpp"

#include <glm/ext.hpp

#include "../../platform.hpp"
#include "../../../device/gpu/gpu.hpp"
#include "../../../physics/collision.hpp"
#include "../objects/gameObject.hpp"

namespace Platform::Game::Components {
    const char* CameraComponent::componentName = "CameraComponent";

    CameraParameters CameraComponent::getParameters(const Device::GPU::GpuViewportType& viewport) const {
        const float aspect = glm::max(glm::epsilon<float>(), static_cast<float>(viewport.width) / glm::max(static_cast<float>(viewport.height), 1.0f));
        const auto& pose = getOwner()->pose;

        CameraParameters parameters;
        parameters.location = pose.location;

        switch (projectionType) {
            case ProjectionType::ORTHOGRAPHIC:
                parameters.projectionMatrix = glm::ortho(viewport.x, viewport.x + viewport.width, viewport.y + viewport.height, viewport.y);
                break;
            case ProjectionType::PERSPECTIVE:
                parameters.projectionMatrix = glm::perspective(fov, aspect, near, far);
                break;
        }

        const glm::vec3 forward = glm::vec3(0, 0, 1) * glm::mat3_cast(pose.rotation);
        const glm::vec3 left = glm::normalize(glm::cross(glm::vec3(0, 1, 0), forward));	// TODO: this can cause a crash if they are parallel!!
        glm::vec3 up = glm::normalize(glm::cross(forward, left));

        // rotate up matrix along forward axis
        up = glm::rotate(glm::angleAxis(roll, forward), up);
        parameters.frustum.set(pose.location, left, up, forward, fov, near, far, aspect);
        parameters.viewMatrix = glm::lookAt(pose.location, pose.location + forward, up);

        return parameters;
    }

    // TODO: there is an inherent problem here in that get_ray will likely be called before
    // the parameters have been calculated properly, and will cause the results of this
    // function to lag one frame behind what's actually on the screen.
    //
    // one solution would be to have components know when their pose (eg. on_pose_changed)
    // has been changed. in this case, we would listen for on_pose_changed, and then, if
    // necessary, we would recalculate the parameters (by calling get_params)
    // and then get the ray as normal. this would be problematic because it would
    // probably add thousands of calls to components that have no business responding
    // to pose changes.
    //
    // the other solution would be to always recalculate the matrices every frame.
    // the calculations are negligible and would only be happening once per frame.
    //
    // another option is to simply have @get_ray accept a viewport, and calculate the
    // matrices before we use them in the unProject functions. this would probably be
    // perfectly acceptable since get_ray wouldn't likely be happening more than
    // once per frame.
    Scene::Structure::Line3<float> CameraComponent::getRay(const Device::GPU::GpuViewportType& viewport, const glm::vec2& screen_location) const {
        // TODO: this is problematic because the viewport isn't necessarily going to be stacked on top, maybe we need to pass in the viewport here?
        Scene::Structure::Line3<float> ray{};
        CameraParameters parameters = getParameters(viewport);
        ray.start = glm::unProject(glm::vec3(screen_location, 0), parameters.viewMatrix, parameters.projectionMatrix, glm::vec4(viewport.x, viewport.y, viewport.width, viewport.height));
        ray.end = glm::unProject(glm::vec3(screen_location, 1), parameters.viewMatrix, parameters.projectionMatrix, glm::vec4(viewport.x, viewport.y, viewport.width, viewport.height));
        return ray;
    }
}