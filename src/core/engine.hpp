#pragma once

#ifndef QUAKE_ENGINE_HPP
#define QUAKE_ENGINE_HPP

#include "window.hpp"
#include "logging/logger.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <map>
#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/length.hpp>
#include <boost/units/base_units/imperial/inch.hpp>

namespace Core {
    class Engine {
    private:
        int width = 1920;
        int height = 1080;
        GLFWwindow* window;
        std::string windowName;

        static void configureCallbacks();
    public:
        std::map<GLFWmonitor*, size_t> monitors;
        std::array<std::unique_ptr<Core::Display>, 8> displays;

        Engine(const char* windowName);

        void init();
        void destroy();

        static void checkEngineInstanceValid();
        static inline void onMonitorEvent(GLFWmonitor* monitor, int event);

        void tick();
        void render();
        bool shouldExit();

        Engine() = default;
    };
}

#endif //QUAKE_ENGINE_HPP
