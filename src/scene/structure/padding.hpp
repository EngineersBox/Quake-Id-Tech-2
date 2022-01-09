#pragma once

#ifndef QUAKE_PADDING_HPP
#define QUAKE_PADDING_HPP

#include <iostream>

#include "aabb.hpp"

namespace Scene {
    namespace Structure {
        template<typename Scalar>
        struct Padding<Scalar, typename std::enable_if<std::is_arithmetic<Scalar>::value>::type> {
            typedef Scalar ScalarType;
            typedef Padding<ScalarType> Type;
            typedef glm::tvec2<ScalarType> VectorType;

            ScalarType bottom = 0;
            ScalarType left = 0;
            ScalarType top = 0;
            ScalarType right = 0;

            Padding() = default;
            Padding(ScalarType bottom, ScalarType left, ScalarType top, ScalarType right) :
                    bottom(bottom),
                    left(left),
                    top(top),
                    right(right) {}

            Padding(ScalarType all) :
                    Padding(all, all, all, all) {}

            ScalarType vertical() const {
                return bottom + top;
            }

            ScalarType horizontal() const {
                return left + right;
            }

            VectorType size() const {
                return VectorType(horizontal(), vertical());
            }

            template<typename U>
            Type operator+(const Padding<U>& rhs) const {
                auto sum = *this;
                sum += rhs;
                return sum;
            }

            template<typename U>
            Type& operator+=(const Padding<U>& rhs) {
                bottom += rhs.bottom;
                left += rhs.left;
                right += rhs.right;
                top += rhs.top;

                return *this;
            }

            template<typename U>
            Type operator-(const Padding<U>& rhs) const {
                auto sum = *this;
                sum -= rhs;
                return sum;
            }

            template<typename U>
            Type& operator-=(const Padding<U>& rhs) {
                bottom -= rhs.bottom;
                left -= rhs.left;
                right -= rhs.right;
                top -= rhs.top;

                return *this;
            }

            template<typename U>
            bool operator==(const Padding<U>& rhs) const {
                return bottom == rhs.bottom && left == rhs.left && right == rhs.right && top == rhs.top;
            }

            template<typename U>
            bool operator!=(const Padding<U>& rhs) const {
                return bottom != rhs.bottom || left != rhs.left || right != rhs.right || top != rhs.top;
            }

            template<typename U>
            operator glm::tvec4<U>() const {
                return glm::tvec4<U>(static_cast<U>(bottom), static_cast<U>(left), static_cast<U>(right), static_cast<U>(top));
            }
        };

        template<typename T>
        std::ostream& operator<<(std::ostream& ostream, const Padding<T>& p) {
            return ostream << "(" << p.bottom << "," << p.left << "," << p.top << "," << p.right << ")";
        }
    }

    typedef Structure::Padding<float> Padding;

    template<typename T, typename U>
    Structure::AABB2<T> operator-(const Structure::AABB2<T>& aabb, const Structure::Padding<U>& padding) {
        return Structure::AABB2<T>(glm::tvec2<T>(aabb.min.x + padding.left, aabb.min.y + padding.bottom), glm::tvec2<T>(aabb.max.x - padding.right, aabb.max.y - padding.top));
    }

    template<typename T, typename U>
    void operator-=(Structure::AABB2<T>& aabb, const Structure::Padding<U>& padding) {
        //TODO: verify correctness
        aabb.min.x += std::min(padding.left, aabb.width());
        aabb.min.y += std::min(padding.bottom, aabb.height());
        aabb.max.x -= std::min(padding.right, aabb.width());
        aabb.max.y -= std::min(padding.top, aabb.height());
    }

    template<typename T, typename U>
    Structure::AABB2<T> operator+(const Structure::AABB2<T>& aabb, const Structure::Padding<U>& padding) {
        return Structure::AABB2<T>(glm::tvec2<T>(aabb.min.x - padding.left, aabb.min.y - padding.bottom), glm::tvec2<T>(aabb.max.x + padding.right, aabb.max.y + padding.top));
    }

    template<typename T, typename U>
    void operator+=(Structure::AABB2<T>& aabb, const Structure::Padding<U>& padding) {
        aabb.min.x -= padding.left;
        aabb.min.y -= padding.bottom;
        aabb.max.x += padding.right;
        aabb.max.y += padding.top;
    }
}

#endif //QUAKE_PADDING_HPP
