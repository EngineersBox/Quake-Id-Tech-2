#include <chrono>
#include <boost/property_tree/json_parser.hpp>

#include "app.hpp"
#include "../../platform/game/game.hpp"

// NOTE: this doesn't seem like the right place to put this
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
#include "../../device/gpu/buffers/gpuBufferManager.hpp"
#include "../../resources/image.hpp"

namespace Core::Application {
    App app;

    void App::run(const boost::shared_ptr<Platform::Game::Game>& _game){

        this->runTimePoint = std::chrono::system_clock::now();
    begin:
        Platform::platform.appRunStart();

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

        Resources::resources.purge();
//        strings.purge();
        Device::GPU::Shaders::shaders.purge();
        Device::GPU::Buffers::gpuBuffers.purge();

        Platform::platform.appRunEnd();

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
        Platform::platform.appTickStart(dt);
        this->game->onTickStart(dt);
//        audio.tick(dt);
        this->game->onTickEnd(dt);
        Platform::platform.appTickEnd(dt);
    }

    void App::render() {
        const auto screenSize = Platform::platform.getScreenSize();

        Device::GPU::gpu.viewports.push(Device::GPU::GpuViewportType(0.0f, 0.0f, screenSize.x, screenSize.y));
        Device::GPU::gpu.clear(Device::GPU::Gpu::CLEAR_FLAG_COLOR | Device::GPU::Gpu::CLEAR_FLAG_DEPTH);

        Platform::platform.appRenderStart();
        this->game->onRenderStart();
        this->game->onRenderEnd();
        Platform::platform.appRenderEnd();

        Device::GPU::gpu.viewports.pop();
    }

    void App::handleInputEvents() {
        Input::InputEvent inputEvent;

        while (Platform::platform.popInputEvent(inputEvent)) {
            if (inputEvent.type.device == Input::InputDeviceType::KEYBOARD &&
                inputEvent.type.key == Input::Keyboard::Key::F11 &&
                inputEvent.type.action == Input::InputActionType::PRESS) {
                Platform::platform.setIsFullscreen(!Platform::platform.isFullscreen());
                continue;
            }
        }
    }

    void App::handleWindowEvents() {
        Core::WindowEvent windowEvent;
        while (Platform::platform.popWindowEvent(windowEvent)) {
            this->game->onWindowEvent(windowEvent);
        }
    }

    bool App::shouldKeepRunning() {
        return !this->isExiting && !this->isResetting && !Platform::platform.shouldExit();
    }
}
