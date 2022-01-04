#pragma once

#ifndef QUAKE_PLATFORMIMPL_HPP
#define QUAKE_PLATFORMIMPL_HPP

#include <map>

#include "platform.hpp"
#include "../device/monitor/monitor.hpp"

struct GLFWwindow;
struct GLFWmonitor;

namespace Platform {
    struct PlatformImpl : PlatformBase {
        PlatformImpl() = default;

        //run
        virtual void appRunStart() override;
        virtual void appRunEnd() override;

        //tick
        virtual void appTickStart(float dt) override;
        virtual void appTickEnd(float dt) override;

        //render
        virtual void appRenderStart() override;
        virtual void appRenderEnd() override;

        //should_exit
        virtual bool shouldExit() const override;

        //screen_size
        virtual glm::vec2 getScreenSize() const override;
        virtual void setScreenSize(const glm::vec2& screen_size) const override;

        //viewport
        virtual Scene::Structure::Rectangle<float> getViewport() const override;

        //fullscreen
        virtual bool isFullscreen() const override;
        virtual void setIsFullscreen(bool is_fullscreen) override;

        //pop_input_event
        virtual bool popInputEvent(Input::InputEvent& input_event) override;

        virtual bool popWindowEvent(Core::WindowEvent& window_event) override;

        //cursor_location
        virtual glm::vec2 getCursorLocation() const override;
        virtual void setCursorLocation(const glm::vec2& cursor_location) const override;

        //is_cursor_hidden
        virtual bool isCursorHidden() const override;
        virtual void setCursorHidden(bool is_hidden) const override;

        //window_title
        virtual std::string getWindowTitle() const override;
        virtual void setWindowTitle(const std::string& window_title) const override;

        //window_size
        virtual glm::vec2 getWindowSize() const override;
        virtual void setWindowSize(const glm::vec2& window_size) const override;

        //window_location
        virtual glm::vec2 getWindowLocation() const override;
        virtual void setWindowLocation(const glm::vec2& window_location) const override;

        //clipboard
        virtual std::string getClipboardString() const override;
        virtual void setClipboardString(const std::string& clipboard) const override;

        std::map<GLFWmonitor*, size_t> monitors;

    private:
        GLFWwindow* window_ptr = nullptr;
        glm::vec2 old_window_size;

        PlatformImpl(const PlatformImpl&) = delete;
        PlatformImpl& operator=(const PlatformImpl&) = delete;
    };

    extern PlatformImpl platform;
}

#endif //QUAKE_PLATFORMIMPL_HPP
