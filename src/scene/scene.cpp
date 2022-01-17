#include "scene.hpp"

#include "../platform/game/components/cameraComponent.hpp"
#include "../platform/game/objects/gameObject.hpp"
#include "../device/gpu/gpu.hpp"
#include "../physics/physicsSimulation.hpp"
#include "../resources/resourceManager.hpp"
#include "../rendering/scene/bsp/bsp.hpp"

namespace Scene {
    Scene::Scene() {
        this->octtree = Logic::Structures::OctTree<float>();
        this->physics = boost::make_shared<Physics::PhysicsSimulation>();
    }

    void Scene::render(const boost::shared_ptr<Device::GPU::Buffers::FrameBuffer>& frame_buffer, const boost::shared_ptr<Platform::Game::Objects::GameObject>& camera) const {
        Device::GPU::GpuViewportType viewport;
        viewport.width = frame_buffer->getSize().x;
        viewport.height = frame_buffer->getSize().y;

        Device::GPU::gpu.viewports.push(viewport);
        Device::GPU::gpu.frameBufferManager.push(frame_buffer);

        Device::GPU::Gpu::Depth::State depth_state = Device::GPU::gpu.depth.getState();
        depth_state.shouldTest = true;
        depth_state.shouldWriteMask = true;
        Device::GPU::gpu.depth.pushState(depth_state);

        Device::GPU::gpu.clear(Device::GPU::Gpu::CLEAR_FLAG_COLOR | Device::GPU::Gpu::CLEAR_FLAG_DEPTH | Device::GPU::Gpu::CLEAR_FLAG_STENCIL);

        if (camera) {
            boost::shared_ptr<Platform::Game::Components::CameraComponent> camera_comp = camera->getComponent<Platform::Game::Components::CameraComponent>("camera");
            if (camera_comp) {
                auto camera_parameters = camera_comp->getParameters(viewport);

                // TODO: this is inefficient, have components
                // register themselves with the scene
                // and only iterate over components that can
                // be drawn
                for (const boost::shared_ptr<Platform::Game::Objects::GameObject>& game_object : gameObjects) {
                    game_object->render(camera_parameters);
                }
            }
        }

        Device::GPU::gpu.depth.popState();
        Device::GPU::gpu.frameBufferManager.pop();
        Device::GPU::gpu.viewports.pop();
    }

    void Scene::tick(float dt) {
        physics->step(dt);

        for (boost::shared_ptr<Platform::Game::Objects::GameObject>& game_object : gameObjects) {
            game_object->onTick(dt);
        }
    }

    void Scene::onInputEvent(Input::InputEvent& input_event) {
        for (boost::shared_ptr<Platform::Game::Objects::GameObject>& game_object : gameObjects) {
            game_object->onInputEvent(input_event);
        }
    }

    boost::shared_ptr<Platform::Game::Objects::GameObject> Scene::createGameObject() {
        boost::shared_ptr<Platform::Game::Objects::GameObject> game_object = boost::make_shared<Platform::Game::Objects::GameObject>();
        game_object->scene = shared_from_this();
        gameObjects.emplace_back(game_object);
        return game_object;
    }


    void Scene::removeGameObject(const boost::shared_ptr<Platform::Game::Objects::GameObject>& game_object) {
        if (game_object->getScene() != shared_from_this()) {
            throw std::runtime_error("");
        }
    }

    Rendering::Query::TraceResult Scene::trace(const glm::vec3& start, const glm::vec3& end) const {
        return physics->trace(start, end);
    }
}