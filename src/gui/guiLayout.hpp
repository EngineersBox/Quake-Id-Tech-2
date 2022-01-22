#pragma once

#include <typeindex>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "guiNode.hpp"
#include "../input/inputEvent.hpp"

namespace GUI {
    struct GUILayout : GUINode {
        template<typename T> requires IsGuiNode<T>
        void put(const std::string& name, boost::shared_ptr<T> node) {
            static const std::type_index TYPE_INDEX = typeid(T);
            auto type_nodes_itr = typeNodes.find(TYPE_INDEX);
            if (type_nodes_itr == typeNodes.end()) {
                typeNodes.insert(typeNodes.begin(), std::make_pair(TYPE_INDEX, std::map<const std::string, boost::shared_ptr<GUINode>>()));
            }

            std::map<const std::string, boost::shared_ptr<GUINode>>& nodes = typeNodes.at(TYPE_INDEX);
            auto nodes_pair = nodes.insert(std::make_pair(name, node));
            if (!nodes_pair.second) {
                throw std::runtime_error("node already exists for type");
            }
        }

        template<typename T> requires IsGuiNode<T>
        boost::shared_ptr<T> get(const std::string& name) {
            static const std::type_index TYPE_INDEX = typeid(T);
            auto type_nodes_itr = typeNodes.find(TYPE_INDEX);
            if (type_nodes_itr == typeNodes.end()) {
                throw std::out_of_range("node doesn't exist for type");
            }

            auto nodes_itr = type_nodes_itr->second.find(name);
            if (nodes_itr == type_nodes_itr->second.end()){
                throw std::out_of_range("node doesn't exist for type");
            }

            return boost::static_pointer_cast<T, GUINode>(nodes_itr->second);
        }

        template<typename T> requires IsGuiNode<T>
        size_t count() {
            static const std::type_index TYPE_INDEX = typeid(T);
            auto type_nodes_itr = typeNodes.find(TYPE_INDEX);
            if (type_nodes_itr == typeNodes.end()) {
                return 0;
            }
            return type_nodes_itr->second.size();
        }

        virtual bool onInputEventBegin(Input::InputEvent& input_event) override;

    private:
        std::map<std::type_index, std::map<const std::string, boost::shared_ptr<GUINode>>> typeNodes;
        std::map<size_t, boost::weak_ptr<GUINode>> touchNodes;
    };
}
