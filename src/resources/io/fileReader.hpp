#pragma once

#ifndef QUAKE_FILEREADER_HPP
#define QUAKE_FILEREADER_HPP

#include <filesystem>
#include <fstream>

namespace Resources::IO {
    [[nodiscard]] static std::string file_contents(const std::filesystem::path& path) {
        if (!std::filesystem::is_regular_file(path)) return {};

        std::ifstream file(path, std::ios::in | std::ios::binary);
        if (!file.is_open()) return {};

        const std::size_t& size = std::filesystem::file_size(path);
        std::string content(size, '\0');
        file.read(content.data(), size);

        file.close();
        return content;
    }
}

#endif //QUAKE_FILEREADER_HPP
