#pragma once

#ifndef QUAKE_IO_HPP
#define QUAKE_IO_HPP

#include <ostream>
#include <array>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <istream>
#include <fstream>

#include "../../scene/structure/range.hpp"

namespace Resources::IO {
    template<typename T>
    inline void read(std::istream& istream, T& t) {
        istream.read(reinterpret_cast<char*>(&t), sizeof(T));
    }

    inline void read(std::istream& istream, std::string& s) {
        std::getline(istream, s, '\0');
    }

    inline void readAllFromFile(const std::string& filePath, std::string& s) {
        std::ifstream in(filePath, std::ios::in | std::ios::binary);
        if (in) {
            s = std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        }
    }

    template<typename T, size_t N>
    inline void read(std::istream& istream, std::array<T, N>& data) {
        istream.read(reinterpret_cast<char*>(data.data()), sizeof(T) * N);
    }

    template<typename T>
    inline void read(std::istream& istream, std::vector<T>& data, size_t count) {
        data.reserve(count);
        std::vector<T> buffer;
        buffer.resize(count);
        istream.read(reinterpret_cast<char*>(buffer.data()), sizeof(T) * count);
        std::copy(buffer.begin(), buffer.end(), std::back_inserter(data));
    }

    template<typename T, size_t N>
    inline void read(std::istream& istream, T data[N]) {
        istream.read(reinterpret_cast<char*>(data), _countof(data) * sizeof(T)); //TODO: same as sizeof(data)?
    }

    template<typename T>
    inline void write(std::ostream& ostream, T& data) {
        ostream.write(reinterpret_cast<char*>(&data), sizeof(T));
    }

    template<typename T>
    inline void write(std::ostream& ostream, const T* data, size_t count) {
        ostream.write(unsafe_cast<char*>(data), count);
    }

    template<typename T>
    inline void write(std::ostream& ostream, std::vector<T>& data) {
        ostream.write(reinterpret_cast<char*>(data.data()), sizeof(T) * data.size());
    }
}

namespace glm {
    template<typename T>
    std::ostream& operator<<(std::ostream& ostream, const glm::tvec2<T>& v) {
        return ostream << "(" << v.x << "," << v.y << ")";
    }

    template<typename T>
    std::ostream& operator<<(std::ostream& ostream, const glm::tvec3<T>& v) {
        return ostream << "(" << v.x << "," << v.y << "," << v.z << ")";
    }

    template<typename T>
    std::ostream& operator<<(std::ostream& ostream, const glm::tvec4<T>& v) {
        return ostream << "(" << v.x << "," << v.y << "," << v.z << "," << v.w << ")";
    }

    template<typename T>
    std::ostream& operator<<(std::ostream& ostream, const glm::tquat<T>& q) {
        return ostream << "(" << q.x << "," << q.y << "," << q.z << "," << q.w << ")";
    }
}

#endif //QUAKE_IO_HPP
