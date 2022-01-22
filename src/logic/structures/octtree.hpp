#pragma once

#ifndef QUAKE_OCTTREE_HPP
#define QUAKE_OCTTREE_HPP

#include <array>
#include <boost/make_shared.hpp>
#include <concepts>

#include "../../scene/structure/aabb.hpp"

namespace Logic {
    namespace Structures {
        template<typename ScalarType> requires std::floating_point<ScalarType>
        struct OctTree {
            using BoundsType = Scene::Structure::AABB3<ScalarType>;
            using Type = OctTree<ScalarType>;
            using ChildrenType = std::array<boost::shared_ptr<Type>, 8>;

            OctTree(ScalarType size = 2048) : bounds(glm::tvec3<ScalarType>(-size / 2), glm::tvec3<ScalarType>(size / 2)) {}

            OctTree(const BoundsType& bounds) :
                    bounds(bounds) {}

            void grow() {
                const auto boundsType = BoundsType(bounds.min, bounds.center());
                const auto childBoundsSize = boundsType.size();

                children[0] = boost::make_shared<Type>(boundsType);
                children[1] = boost::make_shared<Type>(boundsType + BoundsType::VectorType(childBoundsSize.x, 0, 0));
                children[2] = boost::make_shared<Type>(boundsType + BoundsType::VectorType(0, childBoundsSize.y, 0));
                children[3] = boost::make_shared<Type>(boundsType + BoundsType::VectorType(childBoundsSize.x, childBoundsSize.y, 0));
                children[4] = boost::make_shared<Type>(boundsType + BoundsType::VectorType(0, 0, childBoundsSize.z));
                children[5] = boost::make_shared<Type>(boundsType + BoundsType::VectorType(childBoundsSize.x, 0, childBoundsSize.z));
                children[6] = boost::make_shared<Type>(boundsType + BoundsType::VectorType(0, childBoundsSize.y, childBoundsSize.z));
                children[7] = boost::make_shared<Type>(boundsType + childBoundsSize);
            }

            const BoundsType& getBounds() const { return this->bounds; }
            const ChildrenType& getChildren() const { return this->children; }
            bool isLeaf() const { return this->children[0] == nullptr; }

        private:
            BoundsType bounds;
            ChildrenType children;
        };
    }

    typedef Structures::OctTree<float> OctTree;
}

#endif //QUAKE_OCTTREE_HPP
