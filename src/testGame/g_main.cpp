#include "g_main.hpp"
#include "../platform/state/stateSystem.hpp"
#include "../platform/state/state.hpp"
#include "../scene/scene.hpp"

#define GL_SILENCE_DEPRECATION

class TestState : public Platform::States::State, boost::enable_shared_from_this<TestState> {
public:
    TestState(): Platform::States::State() {
        this->isTracing = false;
    }

    void onEnter() {
        super::onEnter();

        Platform::platform.isCursorCentered = true;
        Platform::platform.setCursorHidden(true);
    }
private:
    using super = Platform::States::State;
    bool isTracing;
};

class FreeLookComponent : public Platform::Game::Components::GameComponent {
public:
    FreeLookComponent() {
        this->pitch = 0.0;
        this->yaw = 0.0;
        this->speed = 20.0;
        this->localVelocityTarget = glm::vec3(0, 0, 0);
        this->localVelocity = glm::vec3(0, 0, 0);
        this->sensitivity = 0.125;
        this->pitchMin = -89.0;
        this->pitchMax = 89.0;
    }

    void onInputEvent(Input::InputEvent e) {
        super::onInputEvent(e);
        if (e.type.device == Input::InputDeviceType::MOUSE) {
            handleMouseEvent(e);
        } else if (e.type.device == Input::InputDeviceType::KEYBOARD) {
            handleKeyboardEvent(e);
        }
    }

    void handleMouseEvent(Input::InputEvent e) {
        // TODO
    }

    void handleKeyboardEvent(Input::InputEvent e) {
        // TODO
    }
private:
    float pitch;
    float yaw;
    float speed;
    glm::vec3 localVelocityTarget;
    glm::vec3 localVelocity;
    float sensitivity;
    float pitchMin;
    float pitchMax;

    using super = Platform::Game::Components::GameComponent;
};

class TestGame : public Platform::Game::Game {
public:
    void onRunStart() {
        Platform::platform.windowSize = glm::vec2(1280, 720);
        Platform::States::states.push(boost::make_shared<TestState>(), Platform::States::STATE_FLAG_ALL);

        this->scene = Platform::Scenes::Scene();
        this->camera = this->scene.createGameObject();
        auto platformCamera = Platform::Game::Components::CameraComponent();
        boost::shared_ptr<Platform::Game::Components::CameraComponent> cameraComponent = this->camera->addComponent(
                "camera",
                platformCamera
        );
        auto freelook = FreeLookComponent();
        boost::shared_ptr<FreeLookComponent> freeLookComponent = this->camera->addComponent(
                "freeLook",
                freelook
        );
    }
private:
    Scenes::Scene scene;
    boost::shared_ptr<Platform::Game::Objects::GameObject> camera;
};

int main(int, char**) {
    Core::Logger::init();

    return 0;
}
