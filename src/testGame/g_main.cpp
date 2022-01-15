#include "g_main.hpp"
#include "../platform/state/stateSystem.hpp"

#define GL_SILENCE_DEPRECATION

class TestState : Platform::State::State {
public:
    TestState(): Platform::State::State() {
        this->isTracing = false;
    }

    void onEnter() {
        super::onEnter();

        Platform::platform.isCursorCentered = true;
        Platform::platform.setCursorHidden(true);
    }
private:
    typedef Platform::State::State super;
    bool isTracing;
};

class FreeLookComponent : Platform::Game::Components::GameComponent {
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

    typedef Platform::Game::Components::GameComponent super;
};

class TestGame : Platform::Game::Game {
public:
    void onRunStart() {
        Platform::platform.windowSize = glm::vec2(1280, 720);
        Platform::State::states.push(TestState(), Platform::State::STATE_FLAG_ALL);

        this->scene = Scene::Scene();
        this->camera = this->scene.createGameObject();
        Platform::Game::Components::CameraComponent cameraComponent = this->camera.addComponent(
                "camera",
                Platform::Game::Components::CameraComponent()
        );
        FreeLookComponent freeLookComponent = this->camera.addComponent(
                "freeLook",
                FreeLookComponent()
        );
    }
private:
    Scene::Scene scene;
    Platform::Game::Objects::GameObject camera;
};

int main(int, char**) {
    Core::Logger::init();

    return 0;
}
