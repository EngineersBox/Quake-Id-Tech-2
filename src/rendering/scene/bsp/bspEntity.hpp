#pragma once

#ifndef QUAKE_BSPENTITY_HPP
#define QUAKE_BSPENTITY_HPP

#include <map>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>

namespace Rendering::Scene {
    class BSPEntity {

        [[nodiscard]] const std::string& getClassname() const { return classname; }

    public:
        BSPEntity(const std::string& string);

        template<typename T = std::string>
        boost::optional<T> getOptional(const std::string& key) const {
            boost::optional<T> property = boost::none;
            auto propertiesItr = this->properties.find(key);
            if (propertiesItr != this->properties.end()) {
                try{
                    property = boost::lexical_cast<T>(propertiesItr->second);
                } catch (boost::bad_lexical_cast& e) {}
            }
            return property;
        }

        template<typename T = std::string>
        T get(const std::string& key) const {
            return boost::lexical_cast<T>(this->properties.at(key));
        }

    private:
        std::map<std::string, std::string> properties;
        std::string classname;
    };
}

#endif //QUAKE_BSPENTITY_HPP
