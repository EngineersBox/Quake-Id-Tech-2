#include "platformImpl.hpp"

#include <array>
#include <chrono>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace Platform {
    PlatformImpl platform;

    static inline void onKeyboardKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
        Input::InputEvent input_event;
        input_event.type.device = Input::InputDeviceType::KEYBOARD;
        input_event.type.key = static_cast<Input::Keyboard::Key>(key);

        if ((mods & GLFW_MOD_SHIFT) == GLFW_MOD_SHIFT) {
            input_event.type.flags |= Input::INPUT_EVENT_FLAG_SHIFT;
        }

        if ((mods & GLFW_MOD_CONTROL) == GLFW_MOD_CONTROL) {
            input_event.type.flags |= Input::INPUT_EVENT_FLAG_CTRL;
        }

        if ((mods & GLFW_MOD_ALT) == GLFW_MOD_ALT) {
            input_event.type.flags |= Input::INPUT_EVENT_FLAG_ALT;
        }

        if ((mods & GLFW_MOD_SUPER) == GLFW_MOD_SUPER) {
            input_event.type.flags |= Input::INPUT_EVENT_FLAG_SUPER;
        }

        switch (action) {
            case GLFW_RELEASE:
                input_event.type.action = Input::InputActionType::RELEASE;
                break;
            case GLFW_PRESS:
                input_event.type.action = Input::InputActionType::PRESS;
                break;
            case GLFW_REPEAT:
                input_event.type.action = Input::InputActionType::REPEAT;
                break;
        }

        platform.input.events.push_back(input_event);
    }

    static inline void onKeyboardCharacter(GLFWwindow* window, unsigned int character) {
        Input::InputEvent input_event;
        input_event.type.device = Input::InputDeviceType::KEYBOARD;
        input_event.type.action = Input::InputActionType::CHARACTER;
        input_event.type.character = character;

        platform.input.events.emplace_back(input_event);
    }

    static inline void onMouseButton(GLFWwindow* window, int button, int action, int mods) {
        Input::InputEvent input_event;
        input_event.type.device = Input::InputDeviceType::MOUSE;
        input_event.type.button = button;
        input_event.type.flags = mods;

        bool is_press = (action == GLFW_PRESS);

        input_event.type.action = is_press ? Input::InputActionType::PRESS : Input::InputActionType::RELEASE;

        if (is_press) {
            ++platform.input.touchId;

            input_event.mouse.id = platform.input.touchId;
        } else {
            platform.input.touchId = 0;    // TODO: replace with something better
        }

        input_event.mouse.x = platform.getCursorLocation().x;
        input_event.mouse.y = platform.getCursorLocation().y;

        platform.input.events.push_back(input_event);
    }

    static inline void onMouseMove(GLFWwindow* window, double x, double y) {
        const glm::vec2 screen_size = platform.getScreenSize();

        Input::InputEvent input_event;
        input_event.type.device = Input::InputDeviceType::MOUSE;
        input_event.type.action = Input::InputActionType::MOVE;
        input_event.mouse.x = static_cast<float>(x);
        input_event.mouse.y = static_cast<float>(y);
        input_event.mouse.dx = static_cast<float>(x) - platform.cursorLocation.x;
        input_event.mouse.dy = -(static_cast<float>(y) - platform.cursorLocation.y);

        input_event.mouse.id = platform.input.touchId;

        if (platform.isCursorCentered) {
            const glm::vec2 screen_size = platform.getScreenSize();
            glm::vec2 cursor_location = glm::floor(static_cast<glm::vec2>(screen_size) / 2.0f);
            platform.setCursorLocation(cursor_location);
        }

        platform.cursorLocation = platform.getCursorLocation();
        platform.input.events.push_back(input_event);
    }

    static inline void onMouseScroll(GLFWwindow* window, double x, double y) {
        const glm::vec2 screen_size = platform.getScreenSize();

        Input::InputEvent input_event;
        input_event.type.device = Input::InputDeviceType::MOUSE;
        input_event.type.action = Input::InputActionType::SCROLL;
        input_event.type.axis = static_cast<int>(glm::sign(x));
        input_event.mouse.x = platform.getCursorLocation().x;
        input_event.mouse.y = platform.getCursorLocation().y;
        input_event.mouse.dx = static_cast<float>(x);
        input_event.mouse.dy = static_cast<float>(y);

        platform.input.events.push_back(input_event);
    }

    static inline void onWindowResize(GLFWwindow* window, int width, int height) {
        if (window == nullptr || width == 0 || height == 0) return;

        //TODO: a less verbose solution is possible
        auto window_events_itr = std::find_if(
                platform.window.events.begin(),
                platform.window.events.end(),
                [](const Core::WindowEvent& window_event) -> bool {
                    return window_event.type == Core::WindowEventType::RESIZE;
                }
        );

        if (window_events_itr != platform.window.events.end()) {
            window_events_itr->rectangle.width = width;
            window_events_itr->rectangle.height = height;
        } else {
            Core::WindowEvent window_event;
            window_event.type = Core::WindowEventType::RESIZE;
            window_event.rectangle.width = width;
            window_event.rectangle.height = height;

            platform.window.events.emplace_back(window_event);
        }

        platform.window.rectangle.width = width;
        platform.window.rectangle.height = height;
    }

    static inline void onWindowMove(GLFWwindow* window, int x, int y)
    {
        //TODO: a less vebose solution is possible
        auto window_events_itr = std::find_if(
                platform.window.events.begin(),
                platform.window.events.end(),
                [](const Core::WindowEvent& window_event) -> bool {
                    return window_event.type == Core::WindowEventType::MOVE;
                }
        );

        if (window_events_itr != platform.window.events.end()) {
            window_events_itr->rectangle.x = x;
            window_events_itr->rectangle.y = y;
        } else {
            Core::WindowEvent window_event;
            window_event.type = Core::WindowEventType::MOVE;
            window_event.rectangle.x = x;
            window_event.rectangle.y = y;

            platform.window.events.emplace_back(window_event);
        }

        platform.window.rectangle.x = x;
        platform.window.rectangle.y = y;
    }

    static inline void onMonitorEvent(GLFWmonitor* monitor, int event) {
        using namespace boost::units;

        if (event == GLFW_CONNECTED) {
            std::unique_ptr<PlatformImpl::Display>* display_itr = platform.displays.begin();

            for (; display_itr != platform.displays.end(); ++display_itr) {
                if (*display_itr == nullptr) break;
            }
            if (display_itr == platform.displays.end()) return;

            std::unique_ptr<PlatformImpl::Display>& display = (*display_itr);

            display = std::make_unique<PlatformImpl::Display>();
            display->name = glfwGetMonitorName(monitor);

            //https://en.wikipedia.org/wiki/Pixel_density#Calculation_of_monitor_PPI
            int width = 0;
            int height = 0;
            glfwGetMonitorPhysicalSize(monitor, &width, &height);
            glfwGetMonitorPos(monitor, &display->position.x, &display->position.y);

            glm::vec2 physical_size(static_cast<float>(width) / std::milli::den, static_cast<float>(height) / std::milli::den);
            physical_size *= conversion_factor(si::meter_base_unit::unit_type(), imperial::inch_base_unit::unit_type());

            const GLFWvidmode* video_mode = glfwGetVideoMode(monitor);
            float dp = glm::sqrt(static_cast<float>((video_mode->width * video_mode->width) + (video_mode->height * video_mode->height)));
            float di = glm::sqrt(static_cast<float>((physical_size.x * physical_size.x) + (physical_size.y * physical_size.y)));

            display->ppi = (dp / di);
            int video_mode_count = 0;

            display->videoModes.reserve(video_mode_count);

            const GLFWvidmode* video_mode_ptr = glfwGetVideoModes(monitor, &video_mode_count);

            for (auto i = 0; i < video_mode_count; ++i){
                std::__wrap_iter<Platform::PlatformBase::Display::VideoMode*> video_mode_itr = display->videoModes.emplace(display->videoModes.end());

                video_mode_itr->width = video_mode_ptr->width;
                video_mode_itr->height = video_mode_ptr->height;
                video_mode_itr->bitDepth = video_mode_ptr->blueBits + video_mode_ptr->greenBits + video_mode_ptr->redBits;
                video_mode_itr->refreshRate = video_mode_ptr->refreshRate;

                ++video_mode_ptr;
            }
        } else if (event == GLFW_DISCONNECTED) {
            auto monitors_itr = platform.monitors.find(monitor);
            platform.monitors.erase(monitors_itr);
            platform.displays[monitors_itr->second] = nullptr;
        }
    }

    static inline void onError(int error_code, const char* message) {
        throw std::runtime_error(message);
    }

    void PlatformImpl::appRunStart() {
        //glfw
        glfwInit();

        int monitor_count = 0;
        GLFWmonitor** monitor_ptrs = glfwGetMonitors(&monitor_count);

        if (monitor_ptrs != nullptr) {
            for (int i = 0; i < monitor_count; ++i) {
                onMonitorEvent(monitor_ptrs[i], GLFW_CONNECTED);
            }
        }

        glfwSetErrorCallback(onError);

        window_ptr = glfwCreateWindow(1, 1, "naga", nullptr, nullptr);

        if (window_ptr == nullptr) {
            throw std::runtime_error("Failed to create window!");
        }

        glfwMakeContextCurrent(window_ptr);

        glfwSetKeyCallback(window_ptr, onKeyboardKey);
        glfwSetCharCallback(window_ptr, onKeyboardCharacter);
        glfwSetMouseButtonCallback(window_ptr, onMouseButton);
        glfwSetCursorPosCallback(window_ptr, onMouseMove);
        glfwSetScrollCallback(window_ptr, onMouseScroll);
        glfwSetWindowSizeCallback(window_ptr, onWindowResize);
        glfwSetWindowPosCallback(window_ptr, onWindowMove);
        glfwSetMonitorCallback(onMonitorEvent);

        //glew
        unsigned int glew_init_result = glewInit();

        if (glew_init_result != GLEW_OK) {
            throw std::runtime_error((char*)(glewGetErrorString(glew_init_result)));
        }
    }

    void PlatformImpl::appRunEnd() {
        glfwDestroyWindow(window_ptr);
        glfwTerminate();

        window_ptr = nullptr;
    }

    void PlatformImpl::appTickStart(float dt) {
        glfwPollEvents();
    }

    void PlatformImpl::appTickEnd(float dt) {}

    void PlatformImpl::appRenderStart()
    {
    } // TODO: FINISH THIS STUFF

    void PlatformImpl::app_render_end()
    {
        glfwSwapBuffers(window_ptr);
    }

    bool PlatformImpl::should_exit() const
    {
        return glfwWindowShouldClose(window_ptr) != 0;
    }

    vec2 PlatformImpl::get_screen_size() const
    {
        vec2_i32 screen_size;

        glfwGetWindowSize(window_ptr, &screen_size.x, &screen_size.y);

        return static_cast<vec2>(screen_size);
    }

    void PlatformImpl::set_screen_size(const vec2& screen_size) const
    {
        glfwSetWindowSize(window_ptr, static_cast<int>(screen_size.x), static_cast<int>(screen_size.y));
    }

    // viewport
    Rectangle PlatformImpl::get_viewport() const
    {
        Rectangle viewport;
        viewport.width = get_screen_size().x;
        viewport.height = get_screen_size().y;
        return viewport;
    }

    //fullscreen
    bool PlatformImpl::is_fullscreen() const
    {
        return window_ptr != nullptr && glfwGetWindowMonitor(window_ptr) != nullptr;
    }

    void PlatformImpl::set_is_fullscreen(bool is_fullscreen)
    {
        if (window_ptr == nullptr)
        {
            throw std::exception();
        }

        vec2 window_size;

        if (is_fullscreen == this->is_fullscreen())
        {
            return;
        }

        if (is_fullscreen)
        {
            const auto video_mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

            glfwWindowHint(GLFW_RED_BITS, video_mode->redBits);
            glfwWindowHint(GLFW_GREEN_BITS, video_mode->greenBits);
            glfwWindowHint(GLFW_BLUE_BITS, video_mode->blueBits);
            glfwWindowHint(GLFW_REFRESH_RATE, video_mode->refreshRate);

            old_window_size = get_window_size();

            window_size.x = static_cast<f32>(video_mode->width);
            window_size.y = static_cast<f32>(video_mode->height);
        }
        else
        {
            window_size = old_window_size;
        }

        auto new_window_ptr = glfwCreateWindow(static_cast<int>(window_size.x), static_cast<int>(window_size.y), "naga", is_fullscreen ? glfwGetPrimaryMonitor() : nullptr, window_ptr);
        glfwDestroyWindow(window_ptr);

        window_ptr = new_window_ptr;

        glfwMakeContextCurrent(window_ptr);

        glfwSetKeyCallback(window_ptr, on_keyboard_key);
        glfwSetCharCallback(window_ptr, on_keyboard_character);
        glfwSetMouseButtonCallback(window_ptr, on_mouse_button);
        glfwSetCursorPosCallback(window_ptr, on_mouse_move);
        glfwSetScrollCallback(window_ptr, on_mouse_scroll);
        glfwSetWindowSizeCallback(window_ptr, on_window_resize);
        glfwSetWindowPosCallback(window_ptr, on_window_move);

        //HACK: would use set_window_size, but the callback doesn't get triggered
        on_window_resize(window_ptr, static_cast<int>(window_size.x), static_cast<int>(window_size.y));
    }

    bool PlatformImpl::pop_input_event(InputEvent& input_event)
    {
        if (input.events.empty())
        {
            return false;
        }

        input_event = input.events.front();
        input_event.id = input.event_id;

        input.events.pop_front();

        ++input.event_id;

        return true;
    }

    bool PlatformImpl::pop_window_event(WindowEvent& window_event)
    {
        if (window.events.empty())
        {
            return false;
        }

        window_event = window.events.front();

        window.events.pop_front();

        return true;
    }

    vec2 PlatformImpl::get_cursor_location() const
    {
        vec2_f64 cursor_location;

        glfwGetCursorPos(window_ptr, &cursor_location.x, &cursor_location.y);

        return static_cast<vec2>(cursor_location);
    }

    void PlatformImpl::set_cursor_location(const vec2& cursor_location) const
    {
        glfwSetCursorPos(window_ptr, cursor_location.x, cursor_location.y);
    }

    bool PlatformImpl::is_cursor_hidden() const
    {
        return glfwGetInputMode(window_ptr, GLFW_CURSOR) == GLFW_CURSOR_HIDDEN;
    }

    void PlatformImpl::set_cursor_hidden(bool is_hidden) const
    {
        glfwSetInputMode(window_ptr, GLFW_CURSOR, is_hidden ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
    }

    std::string PlatformImpl::get_window_title() const
    {
        //TODO: implement this
        return std::string();
    }

    void PlatformImpl::set_window_title(const std::string& window_title) const
    {
        glfwSetWindowTitle(window_ptr, window_title.c_str());
    }

    vec2 PlatformImpl::get_window_size() const
    {
        vec2_i32 window_size;

        glfwGetWindowSize(window_ptr, &window_size.x, &window_size.y);

        return static_cast<vec2>(window_size);
    }

    void Platform::set_window_size(const vec2& window_size) const
    {
        glfwSetWindowSize(window_ptr, static_cast<int>(window_size.x), static_cast<int>(window_size.y));
    }

    vec2 PlatformImpl::get_window_location() const
    {
        vec2_i32 window_location;

        glfwGetWindowPos(window_ptr, &window_location.x, &window_location.y);

        return static_cast<vec2>(window_location);
    }

    void PlatformImpl::set_window_location(const vec2& window_location) const
    {
        glfwSetWindowPos(window_ptr, static_cast<int>(window_location.x), static_cast<int>(window_location.y));
    }

    std::string PlatformImpl::get_clipboard_string() const
    {
        return glfwGetClipboardString(window_ptr);
    }

    void PlatformImpl::set_clipboard_string(const std::string& clipboard_string) const
    {
        glfwSetClipboardString(window_ptr, clipboard_string.c_str());
    }
}