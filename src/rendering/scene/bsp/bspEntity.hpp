#pragma once

#ifndef QUAKE_BSPENTITY_HPP
#define QUAKE_BSPENTITY_HPP

#include <map>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>

namespace Rendering::Scene {
    class BSPEntity {
        BSPEntity(const std::string& string);

        [[nodiscard]] const std::string& getClassname() const { return classname; }

        template<typename T = std::string>
        T get(const std::string& key) const {
            return boost::lexical_cast<T>(properties.at(key));
        }

        template<typename T = std::string>
        boost::optional<T> getOptional(const std::string& key) const {
            boost::optional<T> property = boost::none;
            auto properties_itr = properties.find(key);
            if (properties_itr != properties.end()) {
                try{
                    property = boost::lexical_cast<T>(properties_itr->second);
                } catch (boost::bad_lexical_cast& e) {}
            }
            return property;
        }
    private:
        std::map<std::string, std::string> properties;
        std::string classname;
    };
}

#endif //QUAKE_BSPENTITY_HPP
