#pragma once

#include <glm/gtx/optimum_pow.hpp>

#include "arithmeticConcept.hpp"

namespace Scenes {
    namespace Structure {
        template<typename Scalar>
        struct Sphere;

        template<typename Scalar> requires arithmetic<Scalar>
        struct Sphere<Scalar> {
            typedef Scalar ScalarType;
            typedef Sphere<ScalarType> Type;
            typedef glm::tvec3<ScalarType> VectorType;

            VectorType origin;
            ScalarType radius = 0;

            Sphere() = default;
            Sphere(const VectorType& origin, ScalarType radius) :
                    origin(origin),
                    radius(radius) {}

            ScalarType area() const {
                return 4.0f * glm::pi<ScalarType>() * glm::pow2(radius);
            }

            ScalarType volume() const {
                return (4.0f / 3.0f) * glm::pi<ScalarType>() * glm::pow2(radius);
            }

            ScalarType circumfrence() const {
                return 2.0f * glm::pi<ScalarType>() * radius;
            }

            ScalarType diameter() const {
                return radius * 2;
            }
        };
    }

    typedef Structure::Sphere<float> Sphere;
}