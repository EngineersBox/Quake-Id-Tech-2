#pragma once

#include <string>
#include <map>
#include <mutex>
#include <boost/shared_ptr.hpp>

namespace Store {
    struct Cache {
		Cache();
		~Cache();

        [[nodiscard]] std::unique_ptr<std::ifstream> get(const std::string& file_name) const;
        int put_buffer(const std::string& file_name, const void* buffer, size_t count);
        int put(const std::string& file_name, const std::string& contents);
        void erase(const std::string& file_name);
        [[nodiscard]] int checksum(const std::string& file_name) const;
        void purge();
        void write();
        void read();

    private:
        std::map<std::string, int> fileChecksums;
        std::mutex mutex;
    };

	extern Cache cache;
}