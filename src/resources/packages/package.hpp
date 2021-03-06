#pragma once

#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/iostreams/device/mapped_file.hpp>

namespace Resources::Packages {
    struct Package {
        struct File {
			std::string package_name;
            std::string name;
            unsigned int offset = 0;
            unsigned int length = 0;
            unsigned int crc32 = 0;
        };

		typedef std::map<const std::string, File> FilesType;

		Package(const std::string& path);
		Package(Package&& rhs);

        std::string path;
        FilesType files;
        boost::iostreams::mapped_file_source mappedFileSource;

		Package& operator=(Package&& rhs);

    private:
		Package(const Package&) = delete;
		Package& operator=(const Package&) = delete;
    };
}
