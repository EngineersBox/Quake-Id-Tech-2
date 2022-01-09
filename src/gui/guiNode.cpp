#pragma once

#include <algorithm>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "../physics/collision.hpp"
#include "../input/inputEvent.hpp"
#include "guiNode.hpp"
#include "../device/gpu/gpu.hpp"

#if defined(DEBUG)
#include "debug_renderer.hpp"
#endif

namespace GUI {
    void GUINode::orphan() {
        if (parent.get() == nullptr) {
            throw std::runtime_error("cannot orphan a node with no parent");
        }

        auto parent_children_itr = std::find(parent->children.begin(), parent->children.end(), parent);

        if (parent_children_itr == parent->children.end()) {
            throw std::runtime_error("parent does not have this node as a child");
        }

        // remove node from parent's child list
        parent->children.erase(parent_children_itr);

        // TODO: might be able to forego dirtying parent if removing this node doesn't
        // affect positioning of sibling elements or sizing of parent element
        //if (dock_mode != GUIDockMode::NONE)

        // mark parent as dirty
        parent->dirty();
        // unlink from parent
        parent = nullptr;
        // uproot
        layout = nullptr;
        // mark as dirty
        dirty();
    }

    void GUINode::adopt(const boost::shared_ptr<GUINode>& node) {
        if (node == nullptr) {
            throw std::invalid_argument("node cannot be null");
        }

        if (node.get() == this) {
            throw std::invalid_argument("nodes cannot adopt themselves");
        }

        if (node->hasParent()) {
            //orphan node from parent
            node->orphan();
        }

        const auto children_itr = std::find(children.begin(), children.end(), node);

        if (children_itr != children.end())  {
            throw std::runtime_error("node is already a child");
        }

        //add node to child list
        children.push_back(node);
        //set node's new parent
        node->parent = shared_from_this();
        //set node's new root
        node->root = root ? root : shared_from_this();
        //mark as dirty
        dirty();
    }

    void GUINode::clean() {
        std::function<void(boost::shared_ptr<GUINode>&, Scene::Structure::AABB2<float>&)> cleanNode = [&cleanNode](boost::shared_ptr<GUINode>& node, Scene::Structure::AABB2<float>& sibling_bounds) -> void {
            if (node->getVisibility() == GUIVisibility::OMIT) return;

            node->onCleanBegin();
            glm::vec2 absolute_desired_size = node->desiredSize;

            if (node->getSizeModesReal().get_x() == GUISizeMode::RELATIVE) {
                if (node->hasParent()) {
                    absolute_desired_size.x *= node->parent->bounds.size().x - node->parent->padding.horizontal();
                } else {
                    absolute_desired_size.x *= sibling_bounds.size().x;
                }
            }

            if (node->getSizeModesReal().get_y() == GUISizeMode::RELATIVE) {
                if (node->hasParent())
                {
                    absolute_desired_size.y *= node->parent->bounds.size().y - node->parent->padding.vertical();
                } else {
                    absolute_desired_size.y *= sibling_bounds.size().y;
                }
            }

            if (node->getSizeModesReal().get_x() == GUISizeMode::AXIS_SCALE) {
                switch (node->dockMode) {
                    case GUIDockMode::LEFT:
                    case GUIDockMode::RIGHT:
                        absolute_desired_size.x = sibling_bounds.height() * node->desiredSize.x;
                        break;
                    case GUIDockMode::NONE:
                        absolute_desired_size.x *= absolute_desired_size.y;
                        break;
                    default:
                        break;
                }
            } else if (node->getSizeModesReal().get_y() == GUISizeMode::AXIS_SCALE) {
                switch (node->dockMode) {
                    case GUIDockMode::BOTTOM:
                    case GUIDockMode::TOP:
                        absolute_desired_size.y = sibling_bounds.width() * node->desiredSize.y;
                        break;
                    case GUIDockMode::NONE:
                        absolute_desired_size.y *= absolute_desired_size.x;
                        break;
                    default:
                        break;
                }
            }

            switch (node->dockMode) {
                case GUIDockMode::BOTTOM:
                    node->bounds.min = sibling_bounds.min;
                    node->bounds.max.x = sibling_bounds.max.x;
                    node->bounds.max.y = sibling_bounds.min.y + absolute_desired_size.y + node->padding.vertical();
                    node->bounds -= node->margin;
                    sibling_bounds.min.y += node->bounds.height() + node->margin.vertical();
                    break;
                case GUIDockMode::FILL:
                    node->bounds = sibling_bounds - node->margin;
                    break;
                case GUIDockMode::LEFT:
                    node->bounds.min = sibling_bounds.min;
                    node->bounds.max.x = sibling_bounds.min.x + absolute_desired_size.x + node->padding.horizontal();
                    node->bounds.max.y = sibling_bounds.max.y;
                    node->bounds -= node->margin;
                    sibling_bounds.min.x += node->bounds.width() + node->margin.horizontal();
                    break;
                case GUIDockMode::RIGHT:
                    node->bounds.min.x = sibling_bounds.max.x - absolute_desired_size.x - node->padding.horizontal();
                    node->bounds.min.y = sibling_bounds.min.y;
                    node->bounds.max = sibling_bounds.max;
                    node->bounds -= node->margin;
                    sibling_bounds.max.x -= node->bounds.width() + node->margin.horizontal();
                    break;
                case GUIDockMode::TOP:
                    node->bounds.min.x = sibling_bounds.min.x;
                    node->bounds.min.y = sibling_bounds.max.y - absolute_desired_size.y - node->padding.vertical();
                    node->bounds.max = sibling_bounds.max;
                    node->bounds -= node->margin;
                    sibling_bounds.max.y -= node->bounds.height() + node->margin.vertical();
                    break;
                default:
                    node->bounds.min = glm::vec2(0);
                    node->bounds.max = absolute_desired_size;
                    glm::vec2 anchor_location;
                    glm::vec2 anchor_translation;
                    if ((node->anchorFlags & GUI_ANCHOR_FLAG_HORIZONTAL) == GUI_ANCHOR_FLAG_HORIZONTAL) {
                        anchor_location.x = sibling_bounds.min.x + (sibling_bounds.width() / 2);
                        anchor_translation.x = -((absolute_desired_size.x / 2) + ((node->margin.right - node->margin.left) / 2));
                    } else {
                        if ((node->anchorFlags & GUI_ANCHOR_FLAG_LEFT) == GUI_ANCHOR_FLAG_LEFT) {
                            anchor_location.x = sibling_bounds.min.x;
                            anchor_translation.x = node->margin.left;
                        } else if ((node->anchorFlags & GUI_ANCHOR_FLAG_RIGHT) == GUI_ANCHOR_FLAG_RIGHT) {
                            anchor_location.x = sibling_bounds.max.x;
                            anchor_translation.x = -(absolute_desired_size.x + node->margin.right);
                        }
                    }

                    if ((node->anchorFlags & GUI_ANCHOR_FLAG_VERTICAL) == GUI_ANCHOR_FLAG_VERTICAL) {
                        anchor_location.y = sibling_bounds.min.y + (sibling_bounds.height() / 2);
                        anchor_translation.y = -((absolute_desired_size.y / 2) + ((node->margin.top - node->margin.bottom) / 2));
                    } else {
                        if ((node->anchorFlags & GUI_ANCHOR_FLAG_BOTTOM) == GUI_ANCHOR_FLAG_BOTTOM) {
                            anchor_location.y = sibling_bounds.min.y;
                            anchor_translation.y = node->margin.bottom;
                        } else if ((node->anchorFlags & GUI_ANCHOR_FLAG_TOP) == GUI_ANCHOR_FLAG_TOP) {
                            anchor_location.y = sibling_bounds.max.y;
                            anchor_translation.y = -(absolute_desired_size.y + node->margin.top);
                        }
                    }
                    node->bounds += anchor_location + anchor_translation + node->anchorOffset;
                    break;
            }

            // TODO: size to children:
            node->size = node->bounds.size();

            Scene::Structure::AABB2<float> children_bounds = node->bounds - node->padding;
            for (auto child : node->children) {
                cleanNode(child, children_bounds);
            }

            if (node->getSizeModesReal().get_x() == GUISizeMode::RELATIVE) {
                if (node->hasParent()) {
                    absolute_desired_size.x *= node->parent->bounds.size().x - node->parent->padding.horizontal();
                } else {
                    absolute_desired_size.x *= sibling_bounds.size().x;
                }
            }

            if (node->getSizeModesReal().get_y() == GUISizeMode::RELATIVE) {
                if (node->hasParent()) {
                    absolute_desired_size.y *= node->parent->bounds.size().y - node->parent->padding.vertical();
                } else {
                    absolute_desired_size.y *= sibling_bounds.size().y;
                }
            }

            node->isDirty = false;
            node->onCleanEnd();
        };

        // HACK: something is amiss with this way of doing things.
        // margins can be accumulated with every tick unless we
        // pad the bounds passed into clean_node. this seems incorrect,
        // but will work for now.
        Scene::Structure::AABB2<float> paddedBounds = bounds + margin;
        cleanNode(shared_from_this(), paddedBounds);
    }

    void GUINode::tick(float dt) {
        onTickBegin(dt);
        for (auto child : children) {
            child->tick(dt);
        }
        onTickEnd(dt);
    }

    bool GUINode::onInputEvent(Input::InputEvent& input_event) {
        if (visibility == GUIVisibility::OMIT) return false;
        if (onInputEventBegin(input_event)) return true;
        for (auto children_itr = children.begin(); children_itr != children.end(); ++children_itr) {
            if ((*children_itr)->onInputEvent(input_event)) return true;
        }
        return onInputEventEnd(input_event);
    }

    const GUISizeModes GUINode::getSizeModesReal() const {
        auto real_size_modes = sizeModes;
        if (real_size_modes.get_x() == GUISizeMode::INHERIT) {
            if (parent) {
                real_size_modes.set_x(parent->getSizeModesReal().get_x());
            } else {
                real_size_modes.set_x(GUISizeMode::ABSOLUTE);
            }
        }

        if (real_size_modes.get_y() == GUISizeMode::INHERIT) {
            if (parent) {
                real_size_modes.set_y(parent->getSizeModesReal().get_y());
            } else {
                real_size_modes.set_y(GUISizeMode::ABSOLUTE);
            }
        }
        return real_size_modes;
    }

    void GUINode::dirty() {
        if (isDirty) return;
        isDirty = true;
        if (parent) parent->dirty();
    }

    void GUINode::render(glm::mat4 world_matrix, glm::mat4 view_projection_matrix) {
        if (visibility != GUIVisibility::VISIBLE) return;

        //TODO: debug rendering?

        bool should_clip = this->shouldClip; // temporary variable in case value changes during render calls somehow

        if (should_clip) {
            //stencil
            auto gpu_stencil_state = Device::GPU::gpu.stencil.getState();

            gpu_stencil_state.isEnabled = true;

            gpu_stencil_state.function.func = Device::GPU::Gpu::StencilFunction::NEVER;
            gpu_stencil_state.function.ref = 1;
            gpu_stencil_state.function.mask = 0xFF;

            gpu_stencil_state.operations.fail = Device::GPU::Gpu::StencilOperation::REPLACE;
            gpu_stencil_state.operations.zfail = Device::GPU::Gpu::StencilOperation::KEEP;
            gpu_stencil_state.operations.zpass = Device::GPU::Gpu::StencilOperation::KEEP;

            gpu_stencil_state.mask = 0xFF;

            Device::GPU::gpu.stencil.pushState(gpu_stencil_state);

            //color
            auto gpu_color_state = Device::GPU::gpu.color.getState();

            gpu_color_state.mask.r = false;
            gpu_color_state.mask.g = false;
            gpu_color_state.mask.b = false;
            gpu_color_state.mask.a = false;

            Device::GPU::gpu.color.pushState(gpu_color_state);

            //depth
            auto gpu_depth_state = Device::GPU::gpu.depth.getState();

            gpu_depth_state.shouldWriteMask = false;

            Device::GPU::gpu.depth.pushState(gpu_depth_state);

            Device::GPU::gpu.clear(Device::GPU::Gpu::CLEAR_FLAG_DEPTH | Device::GPU::Gpu::CLEAR_FLAG_STENCIL);

            renderRectangle(world_matrix, view_projection_matrix, Scene::Structure::Rectangle<float>(bounds), glm::vec4(1), true);

            Device::GPU::gpu.color.popState();
            Device::GPU::gpu.depth.popState();

            gpu_stencil_state.mask = 0;
            gpu_stencil_state.function.func = Device::GPU::Gpu::StencilFunction::NOTEQUAL;
            gpu_stencil_state.function.ref = 0;
            gpu_stencil_state.function.mask = 0xFF;

            Device::GPU::gpu.stencil.popState();
            Device::GPU::gpu.stencil.pushState(gpu_stencil_state);
        }

        //TODO: configure this to be enable-able in-game
#if defined(DEBUG)
        renderRectangle(world_matrix, view_projection_matrix, Rectangle(bounds), vec4(1.0f, 0.0f, 0.0f, 1.0f));
#endif

        onRenderBegin(world_matrix, view_projection_matrix);

        for (auto& child : children) {
            child->render(world_matrix, view_projection_matrix);
        }

        onRenderEnd(world_matrix, view_projection_matrix);

        if (should_clip) {
            Device::GPU::gpu.stencil.popState();
        }
    }
}
