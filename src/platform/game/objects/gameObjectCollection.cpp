#include "gameObjectCollection.hpp"
#include "gameObject.hpp"

namespace Platform::Game::Objects {
    void GameObjectCollection::add(type& game_object) {
        if (game_object->getScene() != nullptr) {
            throw std::invalid_argument("cannot add a game object that is already in a scene");
        }

        objects.push_back(game_object);
        game_object->scene = shared_from_this();
    }

    void GameObjectCollection::erase(type& game_object) {
        if (game_object->getScene() != shared_from_this()) {
            throw std::runtime_error("")
        }
    }
}