#pragma once

#ifndef QUAKE_HASH_HPP
#define QUAKE_HASH_HPP

#include <string>

#include "FNV.hpp"

namespace Utils {
    template<typename ValueType>
    struct Hash {
        typedef std::string StringType;
        typedef Hash<ValueType> Type;

        Hash(): value(0){}

        explicit Hash(const StringType& string) :
                value(fnv1a<ValueType>(reinterpret_cast<StringType::value_type*>(const_cast<StringType::value_type*>(string.c_str())), string.length())) {}

        Hash(Type&& copy) :
                value(copy.value) {}

        const ValueType& get_value() const {
            return value;
        }

    private:
        ValueType value = ValueType(0);

    public:
        Hash& operator=(const Hash& rhs) {
            value = rhs.value;
            return *this;
        }

        Type& operator=(StringType&& string) {
            value(fnv1a<ValueType>(reinterpret_cast<char*>(const_cast<char*>(string.c_str())), string.length()));
            return *this;
        }

        inline Type& operator=(Type&& copy) {
            value = copy.value;
            return *this;
        }

        inline bool operator==(const Type& rhs) const { return value == rhs.value; }
        inline bool operator!=(const Type& rhs) const { return value != rhs.value; }
        inline bool operator>(const Type& rhs) const { return value > rhs.value; }
        inline bool operator<(const Type& rhs) const { return value < rhs.value; }
        inline bool operator>=(const Type& rhs) const { return value >= rhs.value; }
        inline bool operator<=(const Type& rhs) const { return value <= rhs.value; }
    };

    typedef Utils::Hash<unsigned int> hash_u32;
    typedef Utils::Hash<unsigned long long> hash_u64;
    typedef hash_u32 hash;
}

template<typename T>
std::ostream& operator<<(std::ostream& ostream, const Utils::Hash<T>& hash) {
    return ostream << hash.get_value();
}

#endif //QUAKE_HASH_HPP
