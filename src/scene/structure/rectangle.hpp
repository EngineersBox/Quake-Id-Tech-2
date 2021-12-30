#pragma once

#ifndef QUAKE_RECTANGLE_HPP
#define QUAKE_RECTANGLE_HPP

#include "aabb.hpp"

namespace Scene {
    namespace Structure {
        template<typename Scalar, typename Enable = void>
        struct Rectangle;

        template<typename Scalar>
        struct Rectangle<Scalar, typename std::enable_if<std::is_arithmetic<Scalar>::value>::type> {
            typedef Scalar ScalarType;
            typedef glm::tvec2<ScalarType> VectorType;
            typedef Rectangle<ScalarType> Type;

            ScalarType x = 0;
            ScalarType y = 0;
            ScalarType width = 0;
            ScalarType height = 0;

            Rectangle() = default;

            template<typename _Scalar>
            Rectangle(_Scalar x, _Scalar y, _Scalar width, _Scalar height) :
                    x(static_cast<ScalarType>(x)),
                    y(static_cast<ScalarType>(y)),
                    width(static_cast<ScalarType>(width)),
                    height(static_cast<ScalarType>(height)) {}

            template<typename _Scalar>
            Rectangle(const glm::tvec4<_Scalar>& v) :
                    x(static_cast<ScalarType>(v.x)),
                    y(static_cast<ScalarType>(v.y)),
                    width(static_cast<ScalarType>(v.z)),
                    height(static_cast<ScalarType>(v.w)) {}

            template<typename _Scalar>
            Rectangle(const Structure::AABB2<_Scalar>& aabb) {
                x = static_cast<ScalarType>(aabb.min.x);
                y = static_cast<ScalarType>(aabb.min.y);
                width = static_cast<ScalarType>(aabb.width());
                height = static_cast<ScalarType>(aabb.height());
            }

            ScalarType area() const { return width * height; }
            VectorType center() const { return VectorType(x + (width / 2), y + (height / 2)); }
            VectorType size() const { return VectorType(width, height); }
            VectorType min() const { return VectorType(x, y); }
            VectorType max() const { return VectorType(x + width, y + height); }

            template<typename _Scalar>
            operator glm::tvec4<_Scalar>() const {
                return glm::tvec4<_Scalar>(
                    static_cast<_Scalar>(x),
                    static_cast<_Scalar>(y),
                    static_cast<_Scalar>(width),
                    static_cast<_Scalar>(height)
                );
            }

            template<typename _Scalar>
            operator Rectangle<_Scalar>() const {
                return Rectangle<_Scalar>(
                    static_cast<_Scalar>(x),
                    static_cast<_Scalar>(y),
                    static_cast<_Scalar>(width),
                    static_cast<_Scalar>(height)
                );
            }
        };
    }

    typedef Structure::Rectangle<float> Rectangle;
}

#endif //QUAKE_RECTANGLE_HPP
