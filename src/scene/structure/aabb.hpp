#pragma once

#ifndef QUAKE_AABB_HPP
#define QUAKE_AABB_HPP

#include <array>

#include "plane.hpp"
#include "range.hpp"

#include <glm/gtx/quaternion.hpp>

namespace Scene::Structure {
    template<typename Scalar, typename Enable = void>
    struct Padding;
    template<typename Scalar, typename Enable = void>
    struct AABB2;

    template<typename Scalar> requires std::is_arithmetic_v<Scalar>
    struct AABB2<Scalar>: Range<glm::tvec2<Scalar>> {
        typedef Scalar ScalarType;
        typedef AABB2<ScalarType> Type;
        typedef glm::tvec2<ScalarType> VectorType;

        AABB2() = default;
        AABB2(const VectorType& min, const VectorType& max):
                Range<glm::tvec2<Scalar>>(min, max){}

        inline ScalarType width() const {
            return this->max.x - this->min.x;
        }

        inline ScalarType height() const {
            return this->max.y - this->min.y;
        }

        inline VectorType size() const {
            return this->max - this->min;
        }

        inline VectorType center() const {
            return this->min + (size() / static_cast<ScalarType>(2));
        }

        inline VectorType extents() const {
            return (this->max - this->min) / ScalarType(2);
        }

        std::array<VectorType, 4> getCorners() const {
            return {
                this->min,
                VectorType(this->min.x, this->max.y),
                VectorType(this->max.x, this->min.y),
                this->max
            };
        }

        Type operator-(const VectorType& rhs) const {
            return Type(this->min - rhs, this->max - rhs);
        }

        Type operator-(const Padding<float>& rhs) const {
            return Type(this->min - rhs, this->max - rhs);
        }

        Type& operator-=(const VectorType& rhs) {
            *this = *this - rhs;
            return *this;
        }

        Type& operator-=(const Padding<float>& rhs) {
            *this = *this - rhs;
            return *this;
        }

        Type operator+(const VectorType& rhs) const {
            return Type(this->min + rhs, this->max + rhs);
        }

        Type operator+(const Padding<float>& rhs) const {
            return Type(this->min + rhs, this->max + rhs);
        }

        Type& operator+=(const VectorType& rhs) {
            *this = *this + rhs;
            return *this;
        }

        static Type join(const Type& lhs, const Type& rhs) {
            return Type(glm::min(lhs.min, rhs.min), glm::max(lhs.max, rhs.max));
        }

        Type& join(const Type& rhs) {
            *this = join(*this, rhs);
            return *this;
        }

        template<typename Scalar_>
        bool operator==(const AABB2<Scalar_>& rhs) const {
            return this->min == rhs.min && this->max == rhs.max;
        }

        template<typename Scalar_>
        bool operator!=(const AABB2<Scalar_>& rhs) const {
            return this->min != rhs.min || this->max != rhs.max;
        }

        template<typename Scalar_>
        Type& operator=(const AABB2<Scalar_>& rhs) const {
            this->min = static_cast<VectorType>(rhs.min);
            this->max = static_cast<VectorType>(rhs.max);
            return *this;
        }

        template<typename T>
        operator AABB2<T>() {
            return AABB2<T>(static_cast<typename AABB2<T>::VectorType>(this->min), static_cast<typename AABB2<T>::VectorType>(this->max));
        }

        Type operator<<(const Type& rhs) const {
            return join(*this, rhs);
        }

        Type& operator<<=(const Type& rhs) {
            return this->join(rhs);
        }
    };

    template<typename T, typename Enable = void>
    struct AABB3;

    template<typename Scalar> requires std::is_arithmetic_v<Scalar>
    struct AABB3<Scalar> : Range<glm::tvec3<Scalar>> {
        typedef Scalar ScalarType;
        typedef AABB3<ScalarType> Type;
        typedef Plane3<float> PlaneType;
        typedef glm::tvec3<ScalarType> VectorType;

        static const size_t CORNER_COUNT = 8;
        static const size_t PLANE_COUNT = 6;

        AABB3() = default;
        AABB3(const VectorType& min, const VectorType& max) :
                Range<glm::tvec3<Scalar>>(min, max){}

        inline ScalarType width() const {
            return this->max.x - this->min.x;
        }

        inline ScalarType height() const {
            return this->max.y - this->min.y;
        }

        inline ScalarType depth() const {
            return this->max.z - this->min.z;
        }

        VectorType size() const {
            return this->max - this->min;
        }

        VectorType center() const {
            return this->min + ((this->max - this->min) / ScalarType(2));
        }

        VectorType extents() const {
            return (this->max - this->min) / ScalarType(2);
        }

        [[nodiscard]] std::array<PlaneType, 6> getPlanes() const {
            return {
                { this->min, glm::vec3(1,0,0) }, // RIGHT
                { this->max, glm::vec3(-1,0,0) }, // LEFT
                { this->min, glm::vec3(0,-1,0) }, // DOWN
                { this->max, glm::vec3(0,1,0) }, // UP
                { this->min, glm::vec3(0,0,-1) }, // BACK
                { this->max, glm::vec3(0,0,1) } // FRONT
            };
        }

        std::array<VectorType, 8> getCorners() const {
            return{
                this->min,
                VectorType(this->min.x, this->min.y, this->max.z),
                VectorType(this->min.x, this->max.y, this->min.z),
                VectorType(this->min.x, this->max.y, this->max.z),
                VectorType(this->max.x, this->min.y, this->min.z),
                VectorType(this->max.x, this->min.y, this->max.z),
                VectorType(this->max.x, this->max.y, this->min.z),
                this->max
            };
        }

        Type operator-(const VectorType& rhs) const {
            return Type(this->min - rhs, this->max - rhs);
        }

        Type& operator-=(const VectorType& rhs) {
            *this = *this - rhs;
            return *this;
        }

        Type operator+(const VectorType& rhs) const {
            return Type(this->min + rhs, this->max + rhs);
        }

        Type& operator+=(const VectorType& rhs) {
            *this = *this + rhs;
            return *this;
        }

        static Type join(const Type& lhs, const VectorType& rhs) {
            return Type(glm::min(lhs.min, rhs), glm::max(lhs.max, rhs));
        }

        static Type join(const Type& lhs, const Type& rhs) {
            return Type(glm::min(lhs.min, rhs.min), glm::max(lhs.max, rhs.max));
        };

        Type& join(const VectorType& rhs) {
            *this = join(*this, rhs);
            return *this;
        }

        Type& join(const Type& rhs) {
            *this = join(*this, rhs);
            return *this;
        }

        template<typename U>
        bool operator==(const AABB3<U>& rhs) const {
            return this->min == rhs.min && this->max == rhs.max;
        }

        template<typename U>
        bool operator!=(const AABB3<U>& rhs) const {
            return this->min != rhs.min || this->max != rhs.max;
        }

        template<typename U>
        Type& operator=(const AABB3<U>& rhs) {
            this->min = static_cast<VectorType>(rhs.min);
            this->max = static_cast<VectorType>(rhs.max);
            return *this;
        }

        template<typename PointScalar, size_t N>
        Type& operator=(const std::array<glm::tvec3<PointScalar>, N>& points) {
            this->min = VectorType(std::numeric_limits<ScalarType>::max());
            this->max = VectorType(-std::numeric_limits<ScalarType>::max());

            for (const auto& point : points) {
                *this <<= point;
            }

            return *this;
        }

        Type operator<<(const Type& rhs) const {
            return join(*this, rhs);
        }

        Type operator<<(const VectorType& rhs) const {
            return join(*this, rhs);
        }

        Type& operator<<=(const VectorType& rhs) {
            return this->join(rhs);
        }

        Type& operator<<=(const Type& rhs) {
            return this->join(rhs);
        }

        Type operator<<(const glm::tmat4x4<ScalarType>& rhs) const {
            Type aabb;
            aabb.min = VectorType(std::numeric_limits<ScalarType>::max());
            aabb.max = VectorType(-std::numeric_limits<ScalarType>::max());

            for (const auto& corner : getCorners()) {
                auto corner_transformed = glm::tvec4<ScalarType>(corner, 1);
                corner_transformed = rhs * corner_transformed;

                aabb <<= VectorType(corner_transformed.x, corner_transformed.y, corner_transformed.z);
            }

            return aabb;
        }

        Type& operator<<=(const glm::tmat4x4<ScalarType>& rhs) {
            *this = *this << rhs;

            return *this;
        }
    };
}

#endif //QUAKE_AABB_HPP
