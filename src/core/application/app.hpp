#pragma once

#include <chrono>
#include <array>
#include <deque>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "../../platform/platform.hpp"

namespace Platform::Game {
    struct Game;
}

namespace Input {
    struct InputEvent;
}

namespace Core::Application {
    struct App {
        struct Performance {
            static const auto FRAME_COUNT = 256;

            struct Frame {
                typedef std::chrono::milliseconds DurationType;
                DurationType duration;
                DurationType renderDuration;
                DurationType tickDuration;
                DurationType inputDuration;
                size_t consumedInputEventCount = 0;
                size_t unconsumedInputEventCount = 0;
            };

            std::deque<Frame> frames;
            float fps = 0.0f;
        };

        const Performance& getPerformance() const { return this->performance; }

        App() = default;

        void run(const boost::shared_ptr<Platform::Game::Game>& _game);
        void exit();
        void reset();
        void screenshot();

        float getUptimeSeconds() const;
        long long getUptimeMilliseconds() const;

    private:
        bool isExiting = false;
        bool isResetting = false;
        Performance performance;
        std::chrono::system_clock::time_point runTimePoint;
        boost::shared_ptr<Platform::Game::Game> game;

        App(const App&) = delete;
        App& operator=(const App&) = delete;

        void tick(float dt);
        void render();
        static void handleInputEvents();
        void handleWindowEvents() const;
        bool shouldKeepRunning() const;
    };

    extern App app;
}