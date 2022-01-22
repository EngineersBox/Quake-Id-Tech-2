#pragma once

#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "../scene/structure/aabb.hpp"
#include "../scene/structure/circle.hpp"
#include "../scene/structure/padding.hpp"
#include "../scene/structure/range.hpp"
#include "guiSize.hpp"

namespace Input { struct InputEvent; }

namespace GUI {
    struct GUILayout;

    typedef unsigned char GUIAnchorFlagsType;

    enum : GUIAnchorFlagsType {
        GUI_ANCHOR_FLAG_NONE = 0,
        GUI_ANCHOR_FLAG_BOTTOM = (1 << 0),
        GUI_ANCHOR_FLAG_LEFT = (1 << 1),
        GUI_ANCHOR_FLAG_RIGHT = (1 << 2),
        GUI_ANCHOR_FLAG_TOP = (1 << 3),
        GUI_ANCHOR_FLAG_VERTICAL = (GUI_ANCHOR_FLAG_BOTTOM | GUI_ANCHOR_FLAG_TOP),
        GUI_ANCHOR_FLAG_HORIZONTAL = (GUI_ANCHOR_FLAG_LEFT | GUI_ANCHOR_FLAG_RIGHT),
        GUI_ANCHOR_FLAG_ALL = (GUI_ANCHOR_FLAG_VERTICAL | GUI_ANCHOR_FLAG_HORIZONTAL)
    };

    enum class GUIDockMode {
        NONE,
        BOTTOM,
        FILL,
        LEFT,
        RIGHT,
        TOP
    };

    enum class GUIVisibility {
        OMIT,
        HIDDEN,
        VISIBLE
    };

    struct GUINode : public boost::enable_shared_from_this<GUINode> {
        typedef Scene::Structure::AABB2<float> BoundsType;
        typedef glm::vec2 SizeType;
        typedef glm::vec4 ColorType;

        const std::string& getName() const { return this->name; }
        const boost::shared_ptr<GUINode>& getRoot() const { return this->root; }
		const boost::shared_ptr<GUINode>& getParent() const { return this->parent; }
        GUIDockMode getDockMode() const { return this->dockMode; }
        GUIAnchorFlagsType getAnchorFlags() const { return this->anchorFlags; }
        const glm::vec2& getAnchorOffset() const { return this->anchorOffset; }
		const Scene::Structure::Padding<float>& getPadding() const { return this->padding; }
		const Scene::Structure::Padding<float>& getMargin() const { return this->margin; }
        const GUISizeModes& getSizeModes() const { return this->sizeModes; }
        const GUISizeModes getSizeModesReal() const;
        const SizeType& getDesiredSize() const { return this->desiredSize; }
        const SizeType& getSize() const { return this->size; }

        const ColorType& getColor() const { return this->color; }
        const BoundsType& getBounds() const { return this->bounds; }
        bool getIsDirty() const { return this->isDirty; }
        GUIVisibility getVisibility() const { return this->visibility; }
        bool getShouldClip() const { return this->shouldClip; }
		const std::vector<boost::shared_ptr<GUINode>>& getChildren() const { return this->children; }

        void setDockMode(GUIDockMode dock_mode) { this->dockMode = dock_mode; dirty(); }
        void setAnchorFlags(GUIAnchorFlagsType anchor_flags) { this->anchorFlags = anchor_flags; dirty(); }
        void setAnchorOffset(const glm::vec2& anchor_offset) { this->anchorOffset = anchor_offset; dirty(); }
        void setPadding(const Scene::Structure::Padding<float>& padding) { this->padding = padding; dirty(); }
        void setMargin(const Scene::Structure::Padding<float>& margin) { this->margin = margin; dirty(); }
        void setSize(const SizeType& size) { this->desiredSize = size; dirty(); }
        void setSizeModes(const GUISizeModes size_modes) { this->sizeModes = size_modes; dirty(); }
        void setColor(const ColorType& color) { this->color = color; dirty(); }
        void setBounds(const BoundsType& bounds) { this->bounds = bounds; dirty(); }
        void setVisibility(GUIVisibility visibility) { this->visibility = visibility; dirty(); }
        void setShouldClip(bool should_clip) { this->shouldClip = should_clip; }

        virtual void onCleanBegin() { }
        virtual void onCleanEnd() { }
        virtual void onRenderBegin(glm::mat4& world_matrix, glm::mat4& view_projection_matrix) { }
        virtual void onRenderEnd(glm::mat4& world_matrix, glm::mat4& view_projection_matrix) { }
        virtual void onTickBegin(float dt) { }
        virtual void onTickEnd(float dt) { }
        virtual bool onInputEventBegin(Input::InputEvent& input_event) { return false; }
        virtual bool onInputEventEnd(Input::InputEvent& input_event) { return false; }

        void clean();
        void tick(float dt);
        void render(glm::mat4 world_matrix, glm::mat4 view_projection_matrix);
        bool onInputEvent(Input::InputEvent& input_event);

        bool hasChildren() const { return !children.empty(); }
        bool hasParent() const { return parent.get() != nullptr; }

        void dirty();
        void orphan();
        void adopt(const boost::shared_ptr<GUINode>& child);

    private:
        std::string name;
        boost::shared_ptr<GUINode> root;
        boost::shared_ptr<GUINode> parent;
        std::vector<boost::shared_ptr<GUINode>> children;
        GUIDockMode dockMode = GUIDockMode::NONE;
        GUIAnchorFlagsType anchorFlags = (GUI_ANCHOR_FLAG_TOP | GUI_ANCHOR_FLAG_LEFT);
        SizeType anchorOffset;
        Scene::Structure::Padding<float> padding;
        Scene::Structure::Padding<float> margin;
        SizeType size;
        SizeType desiredSize;
        ColorType color = ColorType(1.0f);
        BoundsType bounds;
        GUIVisibility visibility = GUIVisibility::VISIBLE;
        bool shouldClip = false;
        bool hasFocus = false;
        GUISizeModes sizeModes;
        boost::shared_ptr<GUILayout> layout;
        bool shouldWrapToContentWidth;

    protected:
        bool isDirty = true;
    };

    template<typename T>
    concept IsGuiNode = std::derived_from<T, GUINode>;
}
