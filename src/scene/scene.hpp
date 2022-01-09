#pragma once

#include <vector>
#include <set>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <glm/glm.hpp>

#include "../logic/structures/octtree.hpp"
#include "../platform/game/objects/gameObjectCollection.hpp"
#include "../rendering/query/trace_result.hpp"

namespace Platform::Game::Components::Objects { struct GameObject; }
namespace Device::GPU::Buffers { struct FrameBuffer; }
namespace Input { struct InputEvent; }
namespace Rendering::Scene { struct BSP; }
// TODO: Implement the stuff below
namespace Physics { struct PhysicsSimulation; }
namespace Logic::Structures { struct OctTree; }
namespace Rendering::Query { struct TraceResult; }

namespace Scene {
    struct Scene : boost::enable_shared_from_this<Scene> {
        Scene();

        const std::vector<boost::shared_ptr<Platform::Game::Components::Objects::GameObject>>& get_game_objects() const { return this->gameObjects; }

        void tick(float dt);
        void render(const boost::shared_ptr<Device::GPU::Buffers::FrameBuffer>& frame_buffer, const boost::shared_ptr<Platform::Game::Components::Objects::GameObject>& camera) const;
        void onInputEvent(Input::InputEvent& input_event);

        boost::shared_ptr<Platform::Game::Components::Objects::GameObject> createGameObject();
        void removeGameObject(const boost::shared_ptr<Platform::Game::Components::Objects::GameObject>& game_object);

        const boost::shared_ptr<Physics::PhysicsSimulation>& get_physics() const { return this->physics; }

        Rendering::Query::TraceResult trace(const glm::vec3& start, const glm::vec3& end) const;

    private:
        friend struct GameObject;

        std::vector<boost::shared_ptr<Platform::Game::Components::Objects::GameObject>> gameObjects;

        boost::shared_ptr<Physics::PhysicsSimulation> physics;
        Logic::Structures::OctTree octtree;
        boost::shared_ptr<Rendering::Scene::BSP> bsp;
    };
}