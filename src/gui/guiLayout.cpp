//naga
#include "guiLayout.hpp"
#include "../platform/platform.hpp"

namespace GUI {
    bool GUILayout::onInputEventBegin(Input::InputEvent& input_event) {
        if (input_event.type.device != Input::InputDeviceType::MOUSE) return false;

        input_event.mouse.y = Platform::platform.getScreenSize().y - input_event.mouse.y;
        auto touch_nodes_itr = touchNodes.find(input_event.mouse.id);

        if (touch_nodes_itr == touchNodes.end()) return false;
        if (touch_nodes_itr->second.expired()) {
            touchNodes.erase(touch_nodes_itr);
        } else {
            //pass input event to owning node to handle
            touch_nodes_itr->second.lock()->onInputEvent(input_event);
        }
        return false;
    }
}
