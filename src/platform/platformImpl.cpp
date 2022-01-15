#include "platformImpl.hpp"

#include <array>
#include <chrono>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace Platform {
    PlatformImpl platform;

    static inline void onKeyboardKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
        Input::InputEvent inputEvent;
        inputEvent.type.device = Input::InputDeviceType::KEYBOARD;
        inputEvent.type.key = static_cast<Input::Keyboard::Key>(key);

        if ((mods & GLFW_MOD_SHIFT) == GLFW_MOD_SHIFT) {
            inputEvent.type.flags |= Input::INPUT_EVENT_FLAG_SHIFT;
        }

        if ((mods & GLFW_MOD_CONTROL) == GLFW_MOD_CONTROL) {
            inputEvent.type.flags |= Input::INPUT_EVENT_FLAG_CTRL;
        }

        if ((mods & GLFW_MOD_ALT) == GLFW_MOD_ALT) {
            inputEvent.type.flags |= Input::INPUT_EVENT_FLAG_ALT;
        }

        if ((mods & GLFW_MOD_SUPER) == GLFW_MOD_SUPER) {
            inputEvent.type.flags |= Input::INPUT_EVENT_FLAG_SUPER;
        }

        switch (action) {
            case GLFW_RELEASE:
                inputEvent.type.action = Input::InputActionType::RELEASE;
                break;
            case GLFW_PRESS:
                inputEvent.type.action = Input::InputActionType::PRESS;
                break;
            case GLFW_REPEAT:
                inputEvent.type.action = Input::InputActionType::REPEAT;
                break;
        }

        platform.input.events.push_back(inputEvent);
    }

    static inline void onKeyboardCharacter(GLFWwindow* window, unsigned int character) {
        Input::InputEvent inputEvent;
        inputEvent.type.device = Input::InputDeviceType::KEYBOARD;
        inputEvent.type.action = Input::InputActionType::CHARACTER;
        inputEvent.type.character = character;

        platform.input.events.emplace_back(inputEvent);
    }

    static inline void onMouseButton(GLFWwindow* window, int button, int action, int mods) {
        Input::InputEvent inputEvent;
        inputEvent.type.device = Input::InputDeviceType::MOUSE;
        inputEvent.type.button = button;
        inputEvent.type.flags = mods;

        bool isPress = (action == GLFW_PRESS);
        inputEvent.type.action = isPress ? Input::InputActionType::PRESS : Input::InputActionType::RELEASE;

        if (isPress) {
            ++platform.input.touchId;
            inputEvent.mouse.id = platform.input.touchId;
        } else {
            platform.input.touchId = 0;    // TODO: replace with something better
        }

        inputEvent.mouse.x = platform.getCursorLocation().x;
        inputEvent.mouse.y = platform.getCursorLocation().y;

        platform.input.events.push_back(inputEvent);
    }

    static inline void onMouseMove(GLFWwindow* window, double x, double y) {
        const glm::vec2 screenSize = platform.getScreenSize();

        Input::InputEvent inputEvent;
        inputEvent.type.device = Input::InputDeviceType::MOUSE;
        inputEvent.type.action = Input::InputActionType::MOVE;
        inputEvent.mouse.x = static_cast<float>(x);
        inputEvent.mouse.y = static_cast<float>(y);
        inputEvent.mouse.dx = static_cast<float>(x) - platform.cursorLocation.x;
        inputEvent.mouse.dy = -(static_cast<float>(y) - platform.cursorLocation.y);

        inputEvent.mouse.id = platform.input.touchId;

        if (platform.isCursorCentered) {
            const glm::vec2 screen_size = platform.getScreenSize();
            glm::vec2 cursor_location = glm::floor(static_cast<glm::vec2>(screen_size) / 2.0f);
            platform.setCursorLocation(cursor_location);
        }

        platform.cursorLocation = platform.getCursorLocation();
        platform.input.events.push_back(inputEvent);
    }

    static inline void onMouseScroll(GLFWwindow* window, double x, double y) {
        const glm::vec2 screenSize = platform.getScreenSize();

        Input::InputEvent inputEvent;
        inputEvent.type.device = Input::InputDeviceType::MOUSE;
        inputEvent.type.action = Input::InputActionType::SCROLL;
        inputEvent.type.axis = static_cast<int>(glm::sign(x));
        inputEvent.mouse.x = platform.getCursorLocation().x;
        inputEvent.mouse.y = platform.getCursorLocation().y;
        inputEvent.mouse.dx = static_cast<float>(x);
        inputEvent.mouse.dy = static_cast<float>(y);

        platform.input.events.push_back(inputEvent);
    }

    static inline void onWindowResize(GLFWwindow* window, int width, int height) {
        if (window == nullptr || width == 0 || height == 0) return;

        //TODO: a less verbose solution is possible
        auto windowEventsItr = std::find_if(
                platform.window.events.begin(),
                platform.window.events.end(),
                [](const Core::WindowEvent& window_event) -> bool {
                    return window_event.type == Core::WindowEventType::RESIZE;
                }
        );

        if (windowEventsItr != platform.window.events.end()) {
            windowEventsItr->rectangle.width = width;
            windowEventsItr->rectangle.height = height;
        } else {
            Core::WindowEvent windowEvent;
            windowEvent.type = Core::WindowEventType::RESIZE;
            windowEvent.rectangle.width = width;
            windowEvent.rectangle.height = height;
            platform.window.events.emplace_back(windowEvent);
        }

        platform.window.rectangle.width = width;
        platform.window.rectangle.height = height;
    }

    static inline void onWindowMove(GLFWwindow* window, int x, int y) {
        //TODO: a less vebose solution is possible
        auto windowEventsItr = std::find_if(
                platform.window.events.begin(),
                platform.window.events.end(),
                [](const Core::WindowEvent& window_event) -> bool {
                    return window_event.type == Core::WindowEventType::MOVE;
                }
        );

        if (windowEventsItr != platform.window.events.end()) {
            windowEventsItr->rectangle.x = x;
            windowEventsItr->rectangle.y = y;
        } else {
            Core::WindowEvent windowEvent;
            windowEvent.type = Core::WindowEventType::MOVE;
            windowEvent.rectangle.x = x;
            windowEvent.rectangle.y = y;
            platform.window.events.emplace_back(windowEvent);
        }

        platform.window.rectangle.x = x;
        platform.window.rectangle.y = y;
    }

    static inline void onMonitorEvent(GLFWmonitor* monitor, int event) {
        using namespace boost::units;

        if (event == GLFW_DISCONNECTED) {
            auto monitorsItr = platform.monitors.find(monitor);
            platform.monitors.erase(monitorsItr);
            platform.displays[monitorsItr->second] = nullptr;
        } else if (event != GLFW_CONNECTED) {
            return;
        }

        auto displayItr = platform.displays.begin();

        for (; displayItr != platform.displays.end(); ++displayItr) {
            if (*displayItr == nullptr) break;
        }
        if (displayItr == platform.displays.end()) return;

        std::unique_ptr<PlatformImpl::Display>& display = (*displayItr);
        display = std::make_unique<PlatformImpl::Display>();
        display->name = glfwGetMonitorName(monitor);

        //https://en.wikipedia.org/wiki/Pixel_density#Calculation_of_monitor_PPI
        int width = 0;
        int height = 0;
        glfwGetMonitorPhysicalSize(monitor, &width, &height);
        glfwGetMonitorPos(monitor, &display->position.x, &display->position.y);

        glm::vec2 physicalSize(static_cast<float>(width) / std::milli::den, static_cast<float>(height) / std::milli::den);
        physicalSize *= conversion_factor(si::meter_base_unit::unit_type(), imperial::inch_base_unit::unit_type());

        const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
        float dp = glm::sqrt(static_cast<float>((videoMode->width * videoMode->width) + (videoMode->height * videoMode->height)));
        float di = glm::sqrt(static_cast<float>((physicalSize.x * physicalSize.x) + (physicalSize.y * physicalSize.y)));

        display->ppi = (dp / di);
        int videoModeCount = 0;
        display->videoModes.reserve(videoModeCount);
        const GLFWvidmode* videoModePtr = glfwGetVideoModes(monitor, &videoModeCount);

        for (auto i = 0; i < videoModeCount; ++i){
            auto videoModeItr = display->videoModes.emplace(display->videoModes.end());
            videoModeItr->width = videoModePtr->width;
            videoModeItr->height = videoModePtr->height;
            videoModeItr->bitDepth = videoModePtr->blueBits + videoModePtr->greenBits + videoModePtr->redBits;
            videoModeItr->refreshRate = videoModePtr->refreshRate;
            ++videoModePtr;
        }
    }

    static inline void onError(int error_code, const char* message) {
        throw std::runtime_error(message);
    }

    void PlatformImpl::bindGLFWCallbacks() {
        glfwSetKeyCallback(this->windowPtr, onKeyboardKey);
        glfwSetCharCallback(this->windowPtr, onKeyboardCharacter);
        glfwSetMouseButtonCallback(this->windowPtr, onMouseButton);
        glfwSetCursorPosCallback(this->windowPtr, onMouseMove);
        glfwSetScrollCallback(this->windowPtr, onMouseScroll);
        glfwSetWindowSizeCallback(this->windowPtr, onWindowResize);
        glfwSetWindowPosCallback(this->windowPtr, onWindowMove);
        glfwSetMonitorCallback(onMonitorEvent);
    }

    void PlatformImpl::appRunStart() {
        glfwInit();
        int monitorCount = 0;
        GLFWmonitor** monitorPtrs = glfwGetMonitors(&monitorCount);
        if (monitorPtrs != nullptr) {
            for (int i = 0; i < monitorCount; ++i) {
                onMonitorEvent(monitorPtrs[i], GLFW_CONNECTED);
            }
        }

        glfwSetErrorCallback(onError);
        bindGLFWCallbacks();
        this->windowPtr = glfwCreateWindow(this->windowSize.x, this->windowSize.y, "naga", nullptr, nullptr);
        if (this->windowPtr == nullptr) {
            throw std::runtime_error("Failed to create window!");
        }
        glfwMakeContextCurrent(this->windowPtr);

        unsigned int glewInitResult = glewInit();
        if (glewInitResult != GLEW_OK) {
            throw std::runtime_error((char*)(glewGetErrorString(glewInitResult)));
        }
    }

    void PlatformImpl::appRunEnd() {
        glfwDestroyWindow(this->windowPtr);
        glfwTerminate();
        this->windowPtr = nullptr;
    }

    void PlatformImpl::appTickStart(float dt) {
        glfwPollEvents();
    }

    void PlatformImpl::appTickEnd(float dt) {}

    void PlatformImpl::appRenderStart() {}

    void PlatformImpl::appRenderEnd() {
        glfwSwapBuffers(this->windowPtr);
    }

    bool PlatformImpl::shouldExit() const {
        return glfwWindowShouldClose(this->windowPtr) != 0;
    }

    glm::vec2 PlatformImpl::getScreenSize() const {
        glm::ivec2 screenSize;
        glfwGetWindowSize(this->windowPtr, &screenSize.x, &screenSize.y);
        return static_cast<glm::vec2>(screenSize);
    }

    void PlatformImpl::setScreenSize(const glm::vec2& screenSize) const
    {
        glfwSetWindowSize(this->windowPtr, static_cast<int>(screenSize.x), static_cast<int>(screenSize.y));
    }

    // viewport
    Scene::Structure::Rectangle<float> PlatformImpl::getViewport() const {
        Scene::Structure::Rectangle<float> viewport;
        viewport.width = getScreenSize().x;
        viewport.height = getScreenSize().y;
        return viewport;
    }

    //fullscreen
    bool PlatformImpl::isFullscreen() const {
        return this->windowPtr != nullptr && glfwGetWindowMonitor(this->windowPtr) != nullptr;
    }

    void PlatformImpl::setIsFullscreen(bool isFullscreen) {
        if (this->windowPtr == nullptr) throw std::exception();
        if (isFullscreen == this->isFullscreen()) return;

        glm::vec2 windowSize;
        if (isFullscreen) {
            const GLFWvidmode* videoMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            glfwWindowHint(GLFW_RED_BITS, videoMode->redBits);
            glfwWindowHint(GLFW_GREEN_BITS, videoMode->greenBits);
            glfwWindowHint(GLFW_BLUE_BITS, videoMode->blueBits);
            glfwWindowHint(GLFW_REFRESH_RATE, videoMode->refreshRate);

            oldWindowSize = getWindowSize();
            windowSize.x = static_cast<float>(videoMode->width);
            windowSize.y = static_cast<float>(videoMode->height);
        } else {
            windowSize = oldWindowSize;
        }

        GLFWwindow* newWindowPtr = glfwCreateWindow(static_cast<int>(windowSize.x), static_cast<int>(windowSize.y), "Quake", isFullscreen ? glfwGetPrimaryMonitor() : nullptr, this->windowPtr);
        glfwDestroyWindow(this->windowPtr);

        this->windowPtr = newWindowPtr;
        glfwMakeContextCurrent(this->windowPtr);
        bindGLFWCallbacks();

        //HACK: would use set_window_size, but the callback doesn't get triggered
        onWindowResize(this->windowPtr, static_cast<int>(windowSize.x), static_cast<int>(windowSize.y));
    }

    bool PlatformImpl::popInputEvent(Input::InputEvent& inputEvent) {
        if (input.events.empty()) return false;
        inputEvent = input.events.front();
        inputEvent.id = input.eventId;
        input.events.pop_front();
        ++input.eventId;
        return true;
    }

    bool PlatformImpl::popWindowEvent(Core::WindowEvent& windowEvent) {
        if (window.events.empty()) return false;
        windowEvent = window.events.front();
        window.events.pop_front();
        return true;
    }

    glm::vec2 PlatformImpl::getCursorLocation() const {
        glm::dvec2 cursorLocation;
        glfwGetCursorPos(this->windowPtr, &cursorLocation.x, &cursorLocation.y);
        return static_cast<glm::vec2>(cursorLocation);
    }

    void PlatformImpl::setCursorLocation(const glm::vec2& cursorLocation) const {
        glfwSetCursorPos(this->windowPtr, cursorLocation.x, cursorLocation.y);
    }

    bool PlatformImpl::isCursorHidden() const {
        return glfwGetInputMode(this->windowPtr, GLFW_CURSOR) == GLFW_CURSOR_HIDDEN;
    }

    void PlatformImpl::setCursorHidden(bool is_hidden) const {
        glfwSetInputMode(this->windowPtr, GLFW_CURSOR, is_hidden ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
    }

    std::string PlatformImpl::getWindowTitle() const {
        //TODO: implement this
        return std::string();
    }

    void PlatformImpl::setWindowTitle(const std::string& windowTitle) const {
        glfwSetWindowTitle(this->windowPtr, windowTitle.c_str());
    }

    glm::vec2 PlatformImpl::getWindowSize() const {
        glm::ivec2 windowSize;
        glfwGetWindowSize(this->windowPtr, &windowSize.x, &windowSize.y);
        return static_cast<glm::vec2>(windowSize);
    }

    void PlatformImpl::setWindowSize(const glm::vec2& windowSize) const {
        glfwSetWindowSize(this->windowPtr, static_cast<int>(windowSize.x), static_cast<int>(windowSize.y));
    }

    glm::vec2 PlatformImpl::getWindowLocation() const {
        glm::ivec2 windowLocation;
        glfwGetWindowPos(this->windowPtr, &windowLocation.x, &windowLocation.y);
        return static_cast<glm::vec2>(windowLocation);
    }

    void PlatformImpl::setWindowLocation(const glm::vec2& windowLocation) const {
        glfwSetWindowPos(this->windowPtr, static_cast<int>(windowLocation.x), static_cast<int>(windowLocation.y));
    }

    std::string PlatformImpl::getClipboardString() const {
        return glfwGetClipboardString(this->windowPtr);
    }

    void PlatformImpl::setClipboardString(const std::string& clipboard_string) const {
        glfwSetClipboardString(this->windowPtr, clipboard_string.c_str());
    }
}