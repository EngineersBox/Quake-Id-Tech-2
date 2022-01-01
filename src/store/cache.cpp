#include <fstream>
#include <memory>
#include <sstream>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/crc.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <spdlog/spdlog.h>

#include "cache.hpp"
#include "../resources/io/io.hpp"

#define CACHE_DIRECTORY "cache"
#define CACHE_FILENAME  ".cache"
#define CACHE_PATH      CACHE_DIRECTORY "/" CACHE_FILENAME

namespace Store {
    Cache cache;

	Cache::Cache() {
        read();
    }

	Cache::~Cache() {
        write();
    }

	std::unique_ptr<std::ifstream> Cache::get(const std::string& file_name) const {
        boost::filesystem::path file_path(CACHE_DIRECTORY);
        file_path /= file_name;

        return std::make_unique<std::ifstream>(file_path.string());
    }

	int Cache::put_buffer(const std::string& file_name, const void* data, size_t count) {
        boost::crc_32_type crc32;
        crc32.process_bytes(data, count);

        boost::filesystem::path file_path(CACHE_DIRECTORY);
        file_path /= file_name;

        std::ofstream ofstream(file_path.c_str(), std::ios::out);
        Resources::IO::write(ofstream, data, count);

        unsigned int checksum = crc32.checksum();
        fileChecksums.insert(std::make_pair(file_name, checksum));
        return checksum;
    }

	int Cache::put(const std::string& file_name, const std::string& contents) {
        return put_buffer(file_name, contents.data(), contents.size());
    }

	void Cache::erase(const std::string& file_name) {
        boost::filesystem::path file_path(CACHE_DIRECTORY);
        file_path /= file_name;

        try {
            boost::filesystem::remove(file_path);
        } catch (boost::filesystem::filesystem_error& e) {
            spdlog::error(e.what());
        }
        fileChecksums.erase(file_name);
    }

	void Cache::purge() {
        try {
            boost::filesystem::remove_all(CACHE_DIRECTORY);
        } catch (boost::filesystem::filesystem_error& e) {
            spdlog::error(e.what());
        }
    }

	int Cache::checksum(const std::string& file_name) const {
        auto fileChecksumsItr = fileChecksums.find(file_name);
        if (fileChecksumsItr == fileChecksums.end()) {
            throw std::invalid_argument("file does not exist");
        }
        return fileChecksumsItr->second;
    }

	void Cache::write() {
        std::lock_guard<std::mutex> lock(mutex);
        std::basic_ofstream<char> ofstream = std::ofstream(CACHE_PATH, std::ios::trunc);
        if (!ofstream.is_open()) return;

        boost::property_tree::ptree ptree_;
        for (const auto& fileChecksum : fileChecksums) {
            ptree_.put(fileChecksum.first, fileChecksum.second);
        }

        try {
            write_json(ofstream, ptree_);
        } catch (boost::property_tree::json_parser_error& e) {
            spdlog::error(e.what());
        }
    }

	void Cache::read() {
        std::lock_guard<std::mutex> lock(mutex);
        std::basic_ifstream<char> ifstream = std::ifstream(CACHE_PATH);
        if (!ifstream.is_open()) return ;

        boost::property_tree::ptree ptree_;
        try {
            read_json(ifstream, ptree_);
        } catch (boost::property_tree::json_parser_error& e) {
            std::cerr << e.what() << std::endl;
            return;
        }
        
        fileChecksums.clear();
        for (auto& ptree_itr : ptree_) {
            fileChecksums.insert(std::make_pair(ptree_itr.first, std::atoi(ptree_itr.second.data().c_str())));
        }
    }
}