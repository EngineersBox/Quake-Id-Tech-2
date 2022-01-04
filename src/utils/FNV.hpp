#pragma once

#ifndef QUAKE_FNV_HPP
#define QUAKE_FNV_HPP

namespace Utils {
    template<typename Value>
    struct FNV1A;

    template<>
    struct FNV1A<unsigned int> {
        typedef unsigned int ValueType;

        static const ValueType OFFSET_BASIS = 2166136261UL;
        static const ValueType PRIME = 16777619UL;
    };

    template<>
    struct FNV1A<unsigned long long> {
        typedef unsigned long long ValueType;

        static const ValueType OFFSET_BASIS = 14695981039346656037ULL;
        static const ValueType PRIME = 1099511628211ULL;
    };

    template<typename ValueType>
    ValueType fnv1a(void* ptr, std::size_t length) {
        ValueType hash = Utils::FNV1A<ValueType>::OFFSET_BASIS;
        auto current = static_cast<unsigned char*>(ptr);
        auto end = current + length;

        while (current < end) {
            hash ^= *(current++);
            hash *= Utils::FNV1A<ValueType>::PRIME;
        }

        return hash;
    }
}

#endif //QUAKE_FNV_HPP
