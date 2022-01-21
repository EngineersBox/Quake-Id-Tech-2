#pragma once

#ifndef QUAKE_POSE_HPP
#define QUAKE_POSE_HPP

#include <glm/gtx/matrix_decompose.hpp>

namespace Models {
    template<typename T>
    struct Pose {
        using RotationType = glm::tquat<T>;
        using MatrixType = glm::tmat4x4<T>;
    };

    template<typename T>
    struct Pose2 : Pose<T> {
        using LocationType = glm::tvec2<T>;
        using ScaleType = glm::tvec2<T>;
        using Type = Pose2<T>;

        LocationType location;
        glm::tquat<T> rotation;
        ScaleType scale = ScaleType(1);

        inline glm::tmat4x4<T> toMatrix() const {
            return glm::translate(location.x, location.y, T(0)) * glm::toMat4(rotation) * glm::scale(scale.x, scale.y, 1.0);
        }
    };

    template<typename T>
    struct Pose3 : Pose<T> {
        using LocationType = glm::tvec3<T>;
        using ScaleType = glm::tvec3<T>;
        using Type = Pose3<T>;

        Pose3() = default;
        Pose3(const glm::tmat4x4<T>& matrix) {
            glm::vec3 skew;
            glm::vec4 perspective;
            glm::decompose(matrix, scale, rotation, location, skew, perspective);
            rotation = glm::conjugate(rotation);
        }

        LocationType location;
        glm::tquat<T> rotation;
        ScaleType scale = ScaleType(1);

        inline glm::tmat4x4<T> to_matrix() const {
            return glm::translate(glm::tmat4x4<T>(1), location) * glm::toMat4(rotation) * glm::scale(glm::tmat4x4<T>(1), scale);
        }

        Type& operator *=(const Type& rhs) {
            *this = *this * rhs;
            return *this;
        }
    };

    template<typename T>
    Pose3<T> operator *(const Pose3<T>& lhs, const Pose3<T>& rhs) {
        return Pose3<T>(lhs.to_matrix() * rhs.to_matrix());
    }
}

#endif //QUAKE_POSE_HPP
