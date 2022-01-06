#include <boost/make_shared.hpp>

#include <glm/ext.hpp>

#include "../../device/gpu/opengl.hpp"
#include "state.hpp"
#include "../platform.hpp"
#include "../../device/gpu/gpu.hpp"
#include "../../gui/guiLayout.hpp"
#include "../../core/windowEvent.hpp"


namespace Platform {
    State::State() {
        this->layout = boost::make_shared<GUILayout>();
        this->layout->set_dock_mode(GUIDockMode::FILL);
    }

    void State::tick(float dt) {
        on_tick(dt);

        //TODO: get child nodes to tell layout about cleanliness, recursing every tick is expensive!
        std::function<bool(const boost::shared_ptr<GUINode> &)> isDirty = [&](
                const boost::shared_ptr<GUINode> &node) -> bool {
            if (node->get_is_dirty()) return true;
            for (auto &child: node->get_children()) {
                if (is_dirty(child)) return true;
            }
            return false;
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
        this->layout->render(mat4(), view_projection_matrix);
        Device::GPU::gpu.depth.popState();
    }

    bool State::on_input_event(InputEvent &inputEvent) {
        return this->layout->on_input_event(inputEvent);
    }

    void State::on_enter() {
        this->layout->set_bounds(GUINode::BoundsType(glm::vec2(), static_cast<glm::vec2>(platform.get_screen_size())));
    }

    void State::on_window_event(Core::WindowEvent &windowEvent) {
        if (windowEvent.type == Core::WindowEventType::RESIZE) {
            this->layout->set_bounds(GUINode::BoundsType(glm::vec2(), static_cast<glm::vec2>(window_event.rectangle.size())));
        }
    }
}
