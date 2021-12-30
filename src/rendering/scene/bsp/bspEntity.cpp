#include "bspEntity.hpp"

namespace Rendering::Scene {
    BSPEntity::BSPEntity(const std::string& string) {
        std::string::size_type end = -1;

        for (;;) {
            unsigned long begin = string.find_first_of('"', end + 1);
            if (begin == -1) break;
            end = string.find_first_of('"', begin + 1);

            std::string key = string.substr(begin + 1, end - begin - 1);
            begin = string.find_first_of('"', end + 1);
            end = string.find_first_of('"', begin + 1);

            std::string value = string.substr(begin + 1, end - begin - 1);

            if (key == "classname") {
                classname = value;
            } else {
                properties.insert(std::make_pair(key, value));
            }
        }
    }
}