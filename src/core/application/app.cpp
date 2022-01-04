#include <chrono>
#include <boost/property_tree/json_parser.hpp>

#include "app.hpp"
#include "../../platform/game/game.hpp"
//#include "../../net/httpManager.hpp"

//HACK: this doesn't seem like the right place to put this
//#include "../../device/gpu/shaders/programs/modelShader.hpp"
//#include "../../device/gpu/shaders/programs/guiImageShader.hpp"
#include "../../device/gpu/shaders/programs/bitmapFontShader.hpp"
#include "../../device/gpu/shaders/programs/bspShader.hpp"
#include "../../device/gpu/shaders/programs/blurHorizontalShader.hpp"
#include "../../device/gpu/shaders/programs/basicShader.hpp"
#include "../../device/gpu/shaders/shaderManager.hpp"
#include "../../resources/resourceManager.hpp"
//#include "stringManager.hpp"
//#include "../../device/audio/audioSystem.hpp"
//#include "stateSystem.hpp"
#include "../../device/gpu/buffers/gpuBufferManager.hpp"
#include "../../resources/image.hpp"
#include "../engine.hpp"

namespace Core::Application {
    App app;

    void App::run(const boost::shared_ptr<Platform::Game::Game>& _game){

        this->runTimePoint = std::chrono::system_clock::now();
    begin:
        Core::engine->appRunStart();

//        Device::GPU::Shaders::shaders.make<Device::GPU::Shaders::Programs::GUIImageShader>();
//        Device::GPU::Shaders::shaders.make<Device::GPU::Shaders::Programs::ModelShader>();
        Device::GPU::Shaders::shaders.make<Device::GPU::Shaders::Programs::BitmapFontShader>();
        Device::GPU::Shaders::shaders.make<Device::GPU::Shaders::Programs::BSPShader>();
        Device::GPU::Shaders::shaders.make<Device::GPU::Shaders::Programs::BlurHorizontalShader>();
        Device::GPU::Shaders::shaders.make<Device::GPU::Shaders::Programs::BasicShader>();

        this->game = _game;
        this->game->onRunStart();

        auto simulationTime = std::chrono::high_resolution_clock::now();

        while (shouldKeepRunning()) {
            auto realTime = std::chrono::high_resolution_clock::now();

            handleInputEvents();
            handleWindowEvents();

            // TODO: call serialized callbacks

            while (simulationTime < realTime) {
                static const auto SIMULATION_INTERVAL_DURATION = std::chrono::milliseconds(16);
                static const auto DT = static_cast<float>(SIMULATION_INTERVAL_DURATION.count()) / std::milli::den;
                simulationTime += SIMULATION_INTERVAL_DURATION;
                tick(DT);
            }

            render();

            auto b = std::chrono::high_resolution_clock::now() - realTime;
            long long c = b.count();

            performance.fps = 1.0f / (static_cast<float>(c) / std::chrono::high_resolution_clock::duration::period::den);
        }

        this->game->onRunEnd();
        this->game.reset();

        states.purge();
        states.tick(0); //TODO: hack to avoid exceptions throwing on close due to unreleased state objects, find a better solution later
        Resources::resources.purge();
        strings.purge();
        Device::GPU::Shaders::shaders.purge();
        Device::GPU::Buffers::gpuBuffers.purge();

        platform.appRunEnd();

        if (this->isResetting) {
            this->isResetting = false;
            goto begin;
        }
    }

    void App::exit() {
        this->isExiting = true;
    }

    void App::reset() {
        this->isResetting = true;
    }

    void App::screenshot() {
        int width, height;
        std::unique_ptr<unsigned char[]> pixels = Device::GPU::gpu.getBackbufferPixels(width, height);
        boost::shared_ptr<Resources::Image> image = boost::make_shared<Resources::Image>(
                Resources::Image::SizeType(width, height),
                8,
                Device::GPU::ColorType::RGBA,
                pixels.get(),
                width * height * 4
        );
        const char* filename = "test.png";
        std::ofstream ofstream(filename, std::ios_base::binary);
        if (ofstream.is_open()) {
            ofstream << *image;
        }
    }

    float App::getUptimeSeconds() const {
        using seconds = std::chrono::duration<float>;
        return std::chrono::duration_cast<seconds>(std::chrono::system_clock::now() - this->runTimePoint).count();
    }

    long long App::getUptimeMilliseconds() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - this->runTimePoint).count();
    }

    void App::tick(float dt) {
        platform.appTickStart(dt);
        this->game->onTickStart(dt);
        http.tick();
        states.tick(dt);
        audio.tick(dt);
        this->game->onTickEnd(dt);
        platform.appTickEnd(dt);
    }

    void App::render() {
        const auto screenSize = platform.getScreenSize();

        Device::GPU::gpu.viewports.push(Device::GPU::GpuViewportType(0.0f, 0.0f, screenSize.x, screenSize.y));
        Device::GPU::gpu.clear(Device::GPU::Gpu::CLEAR_FLAG_COLOR | Device::GPU::Gpu::CLEAR_FLAG_DEPTH);

        platform.appRenderStart();
        this->game->onRenderStart();
        states.render();
        this->game->onRenderEnd();
        platform.appRenderEnd();

        Device::GPU::gpu.viewports.pop();
    }

    void App::handleInputEvents() {
        Input::InputEvent inputEvent;

        while (platform.popInputEvent(inputEvent)) {
            if (inputEvent.type.device == Input::InputDeviceType::KEYBOARD &&
                inputEvent.type.key == Input::Keyboard::Key::F11 &&
                inputEvent.type.action == Input::InputActionType::PRESS) {
                platform.setIsFullscreen(!platform.isFullscreen());
                continue;
            }
            //TODO: a bit ugly; if we add more things that evaluate the result
            // of on_input_event, we'll have a nested nightmare. is there a
            // better way to write this?
            // perhaps implement an input_handler interface.
            if (!this->game->onInputEvent(inputEvent)) {
                states.onInputEvent(inputEvent);
            }
        }
    }

    void App::handleWindowEvents() {
        Core::WindowEvent windowEvent;
        while (platform.popWindowEvent(windowEvent)) {
            this->game->onWindowEvent(windowEvent);
            states.onWindowEvent(windowEvent);
        }
    }

    bool App::shouldKeepRunning() {
        return !this->isExiting && !this->isResetting && !platform.shouldExit();
    }
}
