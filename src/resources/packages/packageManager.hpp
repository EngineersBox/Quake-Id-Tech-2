#pragma once

#include <mutex>

#include "package.hpp"

namespace Resources::Packages {
    struct PackageManager {
        void mount(const std::string& path);
        void unmountAll();

        boost::shared_ptr<std::istream> extract(const std::string& fileName);

    private:
        std::recursive_mutex mutex;
        std::map<const std::string, Package::File> files;
		std::map<const std::string, Package> packages;
    };
}
