#pragma once

#ifndef QUAKE_GAMECOMPONENTDEFS_HPP
#define QUAKE_GAMECOMPONENTDEFS_HPP

#include <string>
#include <boost/preprocessor/stringize.hpp>

#define QUAKE_GAME_COMPONENT_DEFINE_COMPONENT_NAME(name)\
    static const char* component_name;\
    virtual std::string getTypeName() const {\
        return std::string(component_name);\
    }

#endif //QUAKE_GAMECOMPONENTDEFS_HPP
