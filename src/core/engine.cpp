#include "engine.hpp"

namespace Core {
    // Static instance for GLFW callback usage since
    // instance reference captures are not allowed
    Engine* engine = nullptr;

    Engine::Engine(const char* windowName) {
        this->windowName = windowName;
        this->window = nullptr;
        engine = this;
    }

    void Engine::init() {
        glfwSetErrorCallback(reinterpret_cast<GLFWerrorfun>(Logger::errorCallbackGLFW));
        glfwInit();
        spdlog::info("Initialised GLFW");

        int monitorCount = 0;
        GLFWmonitor **glfwMonitors = glfwGetMonitors(&monitorCount);
        if (glfwMonitors != nullptr) {
            for (int i = 0; i < monitorCount; i++) {
                Engine::onMonitorEvent(glfwMonitors[i], GLFW_CONNECTED);
            }
        }
        this->window = glfwCreateWindow(this->width, this->height, this->windowName.c_str(), nullptr, nullptr);
        if (this->window == nullptr) {
            throw std::runtime_error("Could not create window with GLFW");
        }
        spdlog::info("Created {0}x{1} GLFW window", this->width, this->height);

        glfwMakeContextCurrent(this->window);
        Engine::configureCallbacks();

        unsigned int glewResult = glewInit();
        if (glewResult != GLEW_OK) {
            throw std::runtime_error(std::string("GLEW Error: ") + reinterpret_cast<const char*>(glewGetErrorString(glewResult)));
        }
        spdlog::info("Initialised GLEW");
    }

    void Engine::configureCallbacks() {
        //    glfwSetKeyCallback(this->window, onKeyboardKey);
        //    glfwSetCharCallback(this->window, onKeyboardCharacter);
        //    glfwSetMouseButtonCallback(this->window, onMouseButton);
        //    glfwSetCursorPosCallback(this->window, onMouseMove);
        //    glfwSetScrollCallback(this->window, onMouseScroll);
        //    glfwSetWindowSizeCallback(this->window, onWindowResize);
        //    glfwSetWindowPosCallback(this->window, onWindowMove);
        glfwSetMonitorCallback(Engine::onMonitorEvent);
        spdlog::debug("Configured GLEW event callback bindings");
    }

    void Engine::destroy() {
        glfwDestroyWindow(this->window);
        glfwTerminate();
        this->window = nullptr;
    }

    void Engine::checkEngineInstanceValid() {
        if (engine == nullptr) {
            throw std::runtime_error("Engine has not been instantiated");
        }
    }

    inline void Engine::onMonitorEvent(GLFWmonitor* monitor, int event) {
        checkEngineInstanceValid();
        if (event == GLFW_DISCONNECTED) {
            auto monitorIter = engine->monitors.find(monitor);
            engine->monitors.erase(monitorIter);
            engine->displays[monitorIter->second] = nullptr;
        } else if (event != GLFW_CONNECTED) {
            return;
        }

        auto displayIter = engine->displays.begin();
        for (; displayIter != engine->displays.end(); ++displayIter) {
            if (*displayIter == nullptr) break;
        }
        if (displayIter == engine->displays.end()) return;

        std::unique_ptr<Platform::PlatformBase::Display>& display = (*displayIter);
        display = std::make_unique<Display>();
        display->name = glfwGetMonitorName(monitor);

        //https://en.wikipedia.org/wiki/Pixel_density#Calculation_of_monitor_PPI
        int width = 0;
        int height = 0;
        glfwGetMonitorPhysicalSize(monitor, &width, &height);
        glfwGetMonitorPos(monitor, &display->position.x, &display->position.y);

        glm::vec2 physicalSize(
                static_cast<float>(width) / std::milli::den,
                static_cast<float>(height) / std::milli::den
        );
        physicalSize *= boost::units::conversion_factor(
                boost::units::si::meter_base_unit::unit_type(),
                boost::units::imperial::inch_base_unit::unit_type()
        );

        const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
        float dp = glm::sqrt(static_cast<float>((videoMode->width * videoMode->width) + (videoMode->height * videoMode->height)));
        float di = glm::sqrt((physicalSize.x * physicalSize.x) + (physicalSize.y * physicalSize.y));
        display->ppi = (dp / di);

        int videoModeCount = 0;
        display->videoModes.reserve(videoModeCount);
        const GLFWvidmode* videoModePtr = glfwGetVideoModes(monitor, &videoModeCount);

        for (int i = 0; i < videoModeCount; ++i) {
            auto video_mode_itr = display->videoModes.emplace(display->videoModes.end());
            video_mode_itr->width = videoModePtr->width;
            video_mode_itr->height = videoModePtr->height;
            video_mode_itr->bitDepth = videoModePtr->blueBits + videoModePtr->greenBits + videoModePtr->redBits;
            video_mode_itr->refreshRate = videoModePtr->refreshRate;
            ++videoModePtr;
        }
    }

    void Engine::tick() {
        glfwPollEvents();
        // Handle other non-standard input devices here (gamepad, joystick, etc)
    }
    void Engine::render() {
        glfwSwapBuffers(this->window);
    }

    bool Engine::shouldExit() {
        return glfwWindowShouldClose(this->window) != 0;
    }
}