#pragma once

#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <concepts>

#include "../../scene/structure/plane.hpp"
#include "../../scene/structure/aabb.hpp"
#include "../../scene/structure/sphere.hpp"

namespace Rendering {
    namespace View {
        enum {
            FRUSTUM_CORNER_INDEX_LEFT_TOP_NEAR,
            FRUSTUM_CORNER_INDEX_LEFT_TOP_FAR,
            FRUSTUM_CORNER_INDEX_LEFT_BOTTOM_NEAR,
            FRUSTUM_CORNER_INDEX_LEFT_BOTTOM_FAR,
            FRUSTUM_CORNER_INDEX_RIGHT_TOP_NEAR,
            FRUSTUM_CORNER_INDEX_RIGHT_TOP_FAR,
            FRUSTUM_CORNER_INDEX_RIGHT_BOTTOM_NEAR,
            FRUSTUM_CORNER_INDEX_RIGHT_BOTTOM_FAR
        };

        enum {
            FRUSTUM_PLANE_INDEX_LEFT,
            FRUSTUM_PLANE_INDEX_RIGHT,
            FRUSTUM_PLANE_INDEX_TOP,
            FRUSTUM_PLANE_INDEX_BOTTOM,
            FRUSTUM_PLANE_INDEX_NEAR,
            FRUSTUM_PLANE_INDEX_FAR
        };

        static const size_t FRUSTUM_PLANE_COUNT = 6;
        static const size_t FRUSTUM_CORNER_COUNT = 8;

        template<typename ScalarType>
        struct Frustum;

        template<typename ScalarType> requires std::floating_point<ScalarType>
        struct Frustum<ScalarType> {
            typedef glm::tvec3<ScalarType> VectorType;
            typedef Frustum<ScalarType> Type;
            typedef ::Scene::Structure::Plane3<ScalarType> PlaneType;
            typedef glm::tmat4x4<ScalarType> MatrixType;
            typedef std::array<PlaneType, FRUSTUM_PLANE_COUNT> PlanesType;
            typedef std::array<VectorType, FRUSTUM_CORNER_COUNT> CornersType;
            typedef ::Scene::Structure::AABB3<ScalarType> AABBType;
            typedef ::Scene::Structure::Sphere<ScalarType> SphereType;

            Frustum() = default;

            void set(const VectorType &origin,
                    const VectorType &left,
                    const VectorType &up,
                    const VectorType &forward,
                     ScalarType fov,
                    ScalarType near,
                    ScalarType far,
                    ScalarType aspect) {
                const glm::vec2 tangent = glm::tan(glm::radians(fov) / 2);
                const auto nearHeight = near * tangent;
                const auto nearWidth = nearHeight * aspect;

                const auto farPlaneHalfHeight = tangent * far;
                const auto nearPlaneHalfHeight = tangent * near;
                const auto farPlaneHalfWidth = farPlaneHalfHeight * aspect;
                const auto nearPlaneHalfWidth = nearPlaneHalfHeight * aspect;

                const auto farPlaneCenter = origin + (forward * far);
                const auto farPlaneRight = farPlaneCenter + (left * farPlaneHalfWidth);
                const auto farPlaneLeft = farPlaneCenter - (left * farPlaneHalfWidth);
                const auto farPlaneTop = farPlaneCenter + (up * farPlaneHalfHeight);
                const auto farPlaneBottom = farPlaneCenter - (up * farPlaneHalfHeight);

                const auto nearPlaneCenter = origin + (forward * near);

                //corners
                this->corners[FRUSTUM_CORNER_INDEX_LEFT_TOP_NEAR] =
                        nearPlaneCenter + (left * nearPlaneHalfWidth) + (up * nearPlaneHalfHeight);
                this->corners[FRUSTUM_CORNER_INDEX_LEFT_TOP_FAR] =
                        farPlaneCenter + (left * farPlaneHalfWidth) + (up * farPlaneHalfHeight);
                this->corners[FRUSTUM_CORNER_INDEX_LEFT_BOTTOM_NEAR] =
                        nearPlaneCenter + (left * nearPlaneHalfWidth) - (up * nearPlaneHalfHeight);
                this->corners[FRUSTUM_CORNER_INDEX_LEFT_BOTTOM_FAR] =
                        farPlaneCenter + (left * farPlaneHalfWidth) - (up * farPlaneHalfHeight);
                this->corners[FRUSTUM_CORNER_INDEX_RIGHT_TOP_NEAR] =
                        nearPlaneCenter - (left * nearPlaneHalfWidth) + (up * nearPlaneHalfHeight);
                this->corners[FRUSTUM_CORNER_INDEX_RIGHT_TOP_FAR] =
                        farPlaneCenter - (left * farPlaneHalfWidth) + (up * farPlaneHalfHeight);
                this->corners[FRUSTUM_CORNER_INDEX_RIGHT_BOTTOM_NEAR] =
                        nearPlaneCenter - (left * nearPlaneHalfWidth) - (up * nearPlaneHalfHeight);
                this->corners[FRUSTUM_CORNER_INDEX_RIGHT_BOTTOM_FAR] =
                        farPlaneCenter - (left * farPlaneHalfWidth) - (up * farPlaneHalfHeight);

                //aabb
                this->aabb = this->corners;

                //planes
                const auto a = glm::normalize(farPlaneRight - origin);
                const auto b = glm::normalize(farPlaneLeft - origin);
                const auto c = glm::normalize(farPlaneTop - origin);
                const auto d = glm::normalize(farPlaneBottom - origin);

                const glm::quat rightPlaneNormal = -glm::cross(up, a);
                const glm::quat leftPlaneNormal = -glm::cross(b, up);
                const glm::quat topPlaneNormal = -glm::cross(c, left);
                const glm::quat bottomPlaneNormal = -glm::cross(left, d);

                this->planes[FRUSTUM_PLANE_INDEX_LEFT] = PlaneType(origin, leftPlaneNormal);
                this->planes[FRUSTUM_PLANE_INDEX_RIGHT] = PlaneType(origin, rightPlaneNormal);
                this->planes[FRUSTUM_PLANE_INDEX_TOP] = PlaneType(origin, topPlaneNormal);
                this->planes[FRUSTUM_PLANE_INDEX_BOTTOM] = PlaneType(origin, bottomPlaneNormal);
                this->planes[FRUSTUM_PLANE_INDEX_NEAR] = PlaneType(nearPlaneCenter, forward);
                this->planes[FRUSTUM_PLANE_INDEX_FAR] = PlaneType(farPlaneCenter, -forward);

                //TODO: verify correctness
                //sphere
                auto viewLength = far - near;
                auto farHeight = viewLength * glm::tan(fov / 2);
                auto farWidth = farHeight;

                //TODO: simplify
                auto p = VectorType(ScalarType(0), ScalarType(0), near + (viewLength / 2));
                auto q = VectorType(farWidth, farHeight, viewLength);
                auto r = q - p;

                this->sphere.radius = glm::length(r);
                this->sphere.origin = origin + (forward * (viewLength / 2) + near);
            }

            const PlanesType &get_planes() const { return this->planes; }

            const CornersType &get_corners() const { return this->corners; }

            const AABBType &get_aabb() const { return this->aabb; }

            const SphereType &get_sphere() const { return this->sphere; }

        private:
            PlanesType planes;
            CornersType corners;
            AABBType aabb;
            SphereType sphere;
        };
    }

    typedef View::Frustum<float> Frustum_f32;
    typedef View::Frustum<double> Frustum_f64;
    typedef Frustum_f32 Frustum;
}