#pragma once

#ifndef QUAKE_GAMECOMPONENTCOLLECTION_HPP
#define QUAKE_GAMECOMPONENTCOLLECTION_HPP

#include <vector>
#include <map>
#include <set>
#include <typeindex>
#include <boost/shared_ptr.hpp>

#include "../../../utils/hash.hpp"
#include "gameComponent.hpp"

namespace Platform::Game {
    struct GameComponentCollection {
        template<typename T, typename std::is_base_of<GameComponent, T>::value = true>
        void add(const boost::shared_ptr<T>& game_component) {
            static std::type_index type_index = typeid(T);
            auto itr = typeIndexComponents.find(type_index);
            if (itr == typeIndexComponents.end()) {}
        }

    private:
        std::map<std::type_index, boost::shared_ptr<GameComponent>> typeIndexComponents;
        std::map<Utils::hash, boost::shared_ptr<GameComponent>> nameComponents;
    };
}

#endif //QUAKE_GAMECOMPONENTCOLLECTION_HPP
