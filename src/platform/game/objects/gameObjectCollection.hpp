#pragma once

#ifndef QUAKE_GAMEOBJECTCOLLECTION_HPP
#define QUAKE_GAMEOBJECTCOLLECTION_HPP

#include <iterator>
#include <vector>
#include <set>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace Platform::Game::Objects {
    struct GameObject;

    struct GameObjectCollection :  public boost::enable_shared_from_this<GameObjectCollection> {
        typedef boost::shared_ptr<GameObject> type;

        void add(type& game_object);
        void erase(type& game_object);
        void find(size_t id);
        void find(type& game_object);

    private:
        std::vector<type> objects;
        std::set<size_t> ids;
    };
}

#endif //QUAKE_GAMEOBJECTCOLLECTION_HPP
