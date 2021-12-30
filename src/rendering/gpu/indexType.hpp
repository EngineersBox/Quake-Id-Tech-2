#pragma once

#ifndef QUAKE_INDEXTYPE_H
#define QUAKE_INDEXTYPE_H

#include <type_traits>
#include <cstdint>

namespace Rendering::GPU {
    template <std::uintmax_t N>
    struct IndexType {
        using Type = std::conditional_t < N <= 255, unsigned char,
            std::conditional_t < N <= 63535, unsigned short,
            std::conditional_t < N <= 4294967295, unsigned int,
            std::conditional_t < N <= 18446744073709551615ULL, unsigned long long,
            std::uintmax_t >> >> ;
    };
}

#endif //QUAKE_INDEXTYPE_H
