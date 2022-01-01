#include "packageManager.hpp"
#include "package.hpp"

#include <sstream>
#include <strstream>
#include <boost/filesystem/path.hpp>
#include <boost/make_shared.hpp>

namespace Resources::Packages {
    void PackageManager::mount(const std::string& path) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        const std::string packageName = boost::filesystem::path(path).filename().string();
		packages.erase(packageName);
		auto packsItr = packages.insert(std::make_pair(packageName, Package(path)));

        if (!packsItr.second) {
            std::ostringstream ostringstream;
			ostringstream << "Package \"" << packageName << "\" already mounted";
            throw std::runtime_error(ostringstream.str().c_str());
        }

        Package& pack = packsItr.first->second;

        for (auto& file : pack.files) {
			file.second.package_name = packageName;
            files[file.first] = file.second;
        }
    }

    void PackageManager::unmountAll() {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        files.clear();
        packages.clear();
    }

	boost::shared_ptr<std::istream> PackageManager::extract(const std::string& file_name) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
		auto filesItr = files.find(file_name);
        if (filesItr == files.end()) {
            std::ostringstream ostringstream;
			ostringstream << "No such file " << file_name;
            throw std::out_of_range(ostringstream.str().c_str());
        }

        const Package::File& file = filesItr->second;
        const Package& package = packages.at(file.package_name);
		return boost::make_shared<std::istrstream>(package.mappedFileSource.data() + file.offset, file.length);
    }
}
