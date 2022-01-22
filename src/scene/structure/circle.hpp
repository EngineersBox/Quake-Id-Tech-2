#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/optimum_pow.hpp>
#include "arithmeticConcept.hpp"

namespace Scenes::Structure {

    template<typename Scalar> requires arithmetic<Scalar>
    struct Circle {
        typedef Scalar ScalarType;
        typedef glm::tvec2<ScalarType> VectorType;
        typedef Circle<Scalar> type;

        VectorType origin;
        ScalarType radius = 0;

        float area() const {
            return glm::pi<float>() * glm::pow2(radius);
        }

        float circumfrence() const {
            return glm::pi<float>() * radius * 2;
        }

        ScalarType diameter() const {
            return radius * 2;
        }
    };
}
