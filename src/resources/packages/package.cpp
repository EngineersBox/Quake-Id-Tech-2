#include "package.hpp"
#include "../io/io.hpp"

#include <sstream>

#define PACK_MAGIC_LENGTH   (4)
#define PACK_MAGIC          (std::array<char, PACK_MAGIC_LENGTH> { { 'P', 'A', 'C', 'K' } })
#define PACK_VERSION        (1)

namespace Resources::Packages {
	Package::Package(const std::string& path) :
        path(path) {
        auto istream = std::ifstream(path, std::ios_base::binary);

        //magic
        std::array<char, PACK_MAGIC_LENGTH> magic;
        IO::read(istream, magic);
        if (magic != PACK_MAGIC) {
            throw std::runtime_error("Invalid package magic string: " + std::string(std::begin(magic), std::end(magic)));
        }

        //version
        unsigned int version = 0;
        IO::read(istream, version);

        if (version != PACK_VERSION) {
            throw std::runtime_error("Invalid package version: " + std::to_string(version));
        }

        //file count
        unsigned int fileCount = 0;
        IO::read(istream, fileCount);

        for(unsigned int i = 0; i < fileCount; ++i) {
            File file;
            std::getline(istream, file.name, '\0');

            IO::read(istream, file.offset);
            IO::read(istream, file.length);
            IO::read(istream, file.crc32);

            auto files_itr = files.emplace(file.name, std::forward<Package::File>(file));

            if (!files_itr.second) {
                throw std::runtime_error("duplicate file in pack");
            }
        }

        this->mappedFileSource = boost::iostreams::mapped_file_source(path);
    }

	Package::Package(Package&& copy) :
        path(std::move(copy.path)),
        files(std::move(copy.files)),
        mappedFileSource(std::move(copy.mappedFileSource)) {}

	Package& Package::operator=(Package&& copy) {
        path = std::move(copy.path);
        files = std::move(copy.files);
        this->mappedFileSource = std::move(copy.mappedFileSource);

        return *this;
    }
}
