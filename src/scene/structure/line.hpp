#pragma once

#ifndef QUAKE_LINE_HPP
#define QUAKE_LINE_HPP

#include <glm/glm.hpp>

namespace Scene::Structure {
    template<typename Scalar, typename Enable = void>
    struct Line2;

    template<typename Scalar> requires std::is_arithmetic_v<Scalar>
    struct Line2<Scalar> {
        using ScalarType = Scalar;
        using Type = Line2<ScalarType>;
        using VectorType = glm::tvec2<ScalarType>;
        using RealType = Line2<float>;

        VectorType start;
        VectorType end;

        Line2() = default;

        Line2(const VectorType& start, const VectorType& end) :
                start(start),
                end(end) {}

        [[nodiscard]] glm::tvec2<float> direction() const {
            return glm::normalize(end - start);
        }

        [[nodiscard]] float length() const {
            return glm::length(end - start);
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

    template<typename Scalar> requires std::is_arithmetic_v<Scalar>
    struct Line3<Scalar> {
        using ScalarType = Scalar;
        using Type = Line3<ScalarType>;
        using VectorType = glm::tvec3<ScalarType>;
        using RealType = Line3<float>;

        VectorType start;
        VectorType end;

        Line3() = default;

        Line3(const VectorType& start, const VectorType& end) :
                start(start),
                end(end) {}

        [[nodiscard]] glm::tvec3<float> direction() const {
            return glm::normalize(end - start);
        }

        [[nodiscard]] float length() const {
            return glm::length(end - start);
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
