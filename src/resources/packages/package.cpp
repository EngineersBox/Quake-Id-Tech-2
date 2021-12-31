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
            throw std::exception();
        }

        //version
        unsigned int version = 0;
        IO::read(istream, version);

        if (version != PACK_VERSION)
        {
            throw std::exception();
        }

        //file count
        unsigned int file_count = 0;
        IO::read(istream, file_count);

        for(unsigned int i = 0; i < file_count; ++i) {
            File file;

            std::getline(istream, file.name, '\0');

            read(istream, file.offset);
            read(istream, file.length);
            read(istream, file.crc32);

            auto files_itr = files.emplace(file.name, std::forward<Package::File>(file));

            if (!files_itr.second) {
                throw std::runtime_error("duplicate file in pack");
            }
        }

        mapped_file_source = boost::iostreams::mapped_file_source(path);
    }

	Package::Package(Package&& copy) :
        path(std::move(copy.path)),
        files(std::move(copy.files)),
        mapped_file_source(std::move(copy.mapped_file_source)) {}

	Package& Package::operator=(Package&& copy) {
        path = std::move(copy.path);
        files = std::move(copy.files);
        mapped_file_source = std::move(copy.mapped_file_source);

        return *this;
    }
}
