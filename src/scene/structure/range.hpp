#pragma once

#ifndef QUAKE_RANGE_HPP
#define QUAKE_RANGE_HPP

namespace Scene::Structure {
    template<typename T>
    struct Range {
        typedef T ValueType;

        Range() = default;
        Range(const ValueType& min, const ValueType& max) :
                min(min),
                max(max) {}

        ValueType min;
        ValueType max;
    };
}

#endif //QUAKE_RANGE_HPP
