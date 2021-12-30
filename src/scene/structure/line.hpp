#pragma once

#ifndef QUAKE_LINE_HPP
#define QUAKE_LINE_HPP

#include <glm/glm.hpp>

namespace Scene::Structure {
    template<typename Scalar, typename Enable = void>
    struct Line2;

    template<typename Scalar>
    struct Line2<Scalar, typename std::enable_if<std::is_arithmetic<Scalar>::value>::type> {
        typedef Scalar ScalarType;
        typedef Line2<ScalarType> Type;
        typedef glm::tvec2<ScalarType> VectorType;
        typedef Line2<float> RealType;

        VectorType start;
        VectorType end;

        Line2() = default;

        Line2(const VectorType& start, const VectorType& end) :
                start(start),
                end(end) {}

        [[nodiscard]] glm::tvec2<float> direction() const {
            return glm::normalize((RealType::VectorType)end - (RealType::VectorType)start);
        }

        [[nodiscard]] float length() const {
            return glm::length((RealType::VectorType)end - (RealType::VectorType)start);
        }

        Type operator-(const VectorType& t) const {
            Type sum;
            sum.start = start - t;
            sum.end = end - t;
            return sum;
        }

        Type& operator-=(const VectorType& t) {
            *this = *this - t;
            return *this;
        }

        Type operator+(const VectorType& t) const {
            Type sum;
            sum.start = start + t;
            sum.end = end + t;
            return sum;
        }

        Type& operator+=(const VectorType& t) {
            *this = *this + t;
            return *this;
        }
    };

    template<typename Scalar, typename Real = void, typename Enable = void>
    struct Line3;

    template<typename Scalar>
    struct Line3<Scalar, typename std::enable_if<std::is_arithmetic<Scalar>::value>::type>
    {
        typedef Scalar ScalarType;
        typedef Line3<ScalarType> Type;
        typedef glm::tvec3<ScalarType> VectorType;
        typedef Line3<float> RealType;

        VectorType start;
        VectorType end;

        Line3() = default;

        Line3(const VectorType& start, const VectorType& end) :
                start(start),
                end(end) {}

        [[nodiscard]] glm::tvec3<float> direction() const {
            return glm::normalize((RealType::VectorType)end - (RealType::VectorType)start);
        }

        [[nodiscard]] float length() const {
            return glm::length((RealType::VectorType)end - (RealType::VectorType)start);
        }

        Type operator-(const VectorType& t) const {
            Type sum;
            sum.start = start - t;
            sum.end = end - t;
            return sum;
        }

        Type& operator-=(const VectorType& t) {
            *this = *this - t;
            return *this;
        }

        Type operator+(const VectorType& t) const {
            Type sum;
            sum.start = start + t;
            sum.end = end + t;
            return sum;
        }

        Type& operator+=(const VectorType& t) {
            *this = *this + t;
            return *this;
        }
    };
}

#endif //QUAKE_LINE_HPP
