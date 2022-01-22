#include <boost/make_shared.hpp>

#include <glm/ext.hpp>
#include <algorithm>

#include "state.hpp"
#include "../platform.hpp"
#include "../../device/gpu/gpu.hpp"
#include "../../gui/guiLayout.hpp"


namespace Platform::States {
    States::State() {
        this->layout = boost::make_shared<GUI::GUILayout>();
        this->layout->setDockMode(GUI::GUIDockMode::FILL);
    }

    void State::tick(float dt) {
        onTick(dt);

        //TODO: get child nodes to tell layout about cleanliness, recursing every tick is expensive!
        std::function<bool(const boost::shared_ptr<GUI::GUINode> &)> isDirty = [&](const boost::shared_ptr<GUI::GUINode> &node) -> bool {
            if (node->getIsDirty()) return true;
            return std::any_of(
                node->getChildren().begin(),
                node->getChildren().end(),
                isDirty
            );
        };
        if (isDirty(layout)) {
            this->layout->clean();
        }
        this->layout->tick(dt);
    }

    void State::render() {
        const glm::vec2 screen_size = platform.getScreenSize();
        glm::mat4 view_projection_matrix = glm::ortho(
            0.0f,
            static_cast<float>(screen_size.x),
            0.0f,
            static_cast<float>(screen_size.y)
        );
        Device::GPU::Gpu::Depth::State depthState = Device::GPU::gpu.depth.getState();
        depthState.shouldTest = false;
        Device::GPU::gpu.depth.pushState(depthState);
        this->layout->render(glm::mat4(), view_projection_matrix);
        Device::GPU::gpu.depth.popState();
    }

    bool State::onInputEvent(Input::InputEvent &input_event) {
        return this->layout->onInputEvent(input_event);
    }

    void State::onEnter() {
        this->layout->setBounds(GUI::GUINode::BoundsType(glm::vec2(), static_cast<glm::vec2>(platform.getScreenSize())));
    }

    void State::onWindowEvent(Core::WindowEvent &window_event) {
        if (window_event.type == Core::WindowEventType::RESIZE) {
            this->layout->setBounds(GUI::GUINode::BoundsType(glm::vec2(), static_cast<glm::vec2>(window_event.rectangle.size())));
        }
    }
}
