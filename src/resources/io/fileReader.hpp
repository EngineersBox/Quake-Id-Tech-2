#pragma once

#ifndef QUAKE_FILEREADER_HPP
#define QUAKE_FILEREADER_HPP

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/string_file.hpp>

namespace Resources::IO {
    [[nodiscard]] static std::string readFile(const boost::filesystem::path& path) {
        std::string fileContents{};
        boost::filesystem::load_string_file(path, fileContents);
        return fileContents;
    }
}

#endif //QUAKE_FILEREADER_HPP
