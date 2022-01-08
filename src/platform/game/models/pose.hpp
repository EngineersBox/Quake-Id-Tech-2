#pragma once

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Platform::Game {
    namespace Models {
        template<typename T>
        struct Pose {
            typedef glm::tquat<T> RotationType;
            typedef glm::tmat4x4<T> MatrixType;
        };

        template<typename T>
        struct Pose2 : Pose<T> {
            typedef glm::tvec2<T> LocationType;
            typedef glm::tvec2<T> ScaleType;
            typedef typename Pose<T>::RotationType RotationType;
            typedef typename Pose<T>::MatrixType MatrixType;
            typedef Pose2<T> Type;

            LocationType location;
            RotationType rotation;
            ScaleType scale = ScaleType(1);

            inline MatrixType to_matrix() const {
                return glm::translate(location.x, location.y, T(0)) * glm::toMat4(rotation) * glm::scale(scale.x, scale.y, 1.0);
            }
        };

        template<typename T>
        struct Pose3 : Pose<T> {
            typedef glm::tvec3<T> LocationType;
            typedef glm::tvec3<T> ScaleType;
            typedef typename Pose<T>::RotationType RotationType;
            typedef typename Pose<T>::MatrixType MatrixType;
            typedef Pose3<T> Type;

            Pose3() = default;
            Pose3(const MatrixType& matrix) {
                glm::vec3 skew;
                glm::vec4 perspective;
                glm::decompose(matrix, scale, rotation, location, skew, perspective);
                rotation = glm::conjugate(rotation);
            }

            LocationType location;
            RotationType rotation;
            ScaleType scale = ScaleType(1);

            inline MatrixType to_matrix() const {
                return glm::translate(MatrixType(1), location) * glm::toMat4(rotation) * glm::scale(MatrixType(1), scale);
            }

            Type& operator *=(const Type& rhs) {
                *this = *this * rhs;
                return *this;
            }
        };
    }

    template<typename T>
    Models::Pose3<T> operator *(const Models::Pose3<T>& lhs, const Models::Pose3<T>& rhs) {
        return Models::Pose3<T>(lhs.to_matrix() * rhs.to_matrix());
    }
}