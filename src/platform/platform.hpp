#pragma once

#ifndef QUAKE_PLATFORM_HPP
#define QUAKE_PLATFORM_HPP

#include <deque>
#include <array>
#include <vector>
#include <memory>
#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/length.hpp>
#include <boost/units/base_units/imperial/inch.hpp>

#include "../input/inputEvent.hpp"
#include "../core/windowEvent.hpp"
#include "../scene/structure/rectangle.hpp"

namespace Platform {
    struct PlatformBase {
        struct InputManager {
            std::deque<Input::InputEvent> events;
            size_t eventId = 0;
            size_t touchId = 1;
        } input;

        struct WindowManager {
            std::deque<Core::WindowEvent> events;
            Scenes::Structure::Rectangle<int> rectangle;
        } window;

        struct Display {
            struct VideoMode {
                int width = 0;
                int height = 0;
                int bitDepth = 0;
                int refreshRate = 0;
            };

            std::string name;
            glm::ivec2 position;
            float ppi = 0.0f;
            std::vector<VideoMode> videoModes;
        };

        //run
        virtual void appRunStart() = 0;
        virtual void appRunEnd() = 0;

        //tick
        virtual void appTickStart(float dt) = 0;
        virtual void appTickEnd(float dt) = 0;

        //render
        virtual void appRenderStart() = 0;
        virtual void appRenderEnd() = 0;

        //should_exit
        virtual bool shouldExit() const = 0;

        //screen_size
        virtual glm::vec2 getScreenSize() const = 0;
        virtual void setScreenSize(const glm::vec2& screen_size) const = 0;

        //viewport
        virtual Scenes::Structure::Rectangle<float> getViewport() const = 0;

        //pop_input_event
        virtual bool popInputEvent(Input::InputEvent& input_event) = 0;

        //clipboard
        virtual std::string getClipboardString() const = 0;
        virtual void setClipboardString(const std::string& clipboard) const = 0;

        //cursor_location
        virtual glm::vec2 getCursorLocation() const = 0;
        virtual void setCursorLocation(const glm::vec2& cursor_location) const = 0;

        //is_cursor_hidden
        virtual bool isCursorHidden() const = 0;
        virtual void setCursorHidden(bool hide_cursor) const = 0;

        virtual std::string getWindowTitle() const = 0;
        virtual void setWindowTitle(const std::string& window_title) const = 0;

        virtual glm::vec2 getWindowSize() const = 0;
        virtual void setWindowSize(const glm::vec2& window_size) const = 0;

        virtual glm::vec2 getWindowLocation() const = 0;
        virtual void setWindowLocation(const glm::vec2& window_location) const = 0;

        virtual bool isFullscreen() const = 0;
        virtual void setIsFullscreen(bool is_fullscreen) = 0;

        //pop_window_event
        virtual bool popWindowEvent(Core::WindowEvent& window_event) = 0;

        bool isCursorCentered = false;
        glm::vec2 cursorLocation;

        [[nodiscard]] const Display& getDisplay(size_t index) const;

        std::array<std::unique_ptr<Display>, 8> displays;
        glm::vec2 windowSize = glm::vec2(640, 480);
    };
}
#include "platformImpl.hpp"
#endif //QUAKE_PLATFORM_HPP
