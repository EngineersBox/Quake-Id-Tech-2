#if defined(DEBUG)
//std
#include <vector>
#include <boost\weak_ptr.hpp>
#endif

#include "resourceManager.hpp"

namespace Resources {
    ResourceManager resources;

	size_t ResourceManager::count() const {
        //TODO: this could be replaced, whenever a resource is added or removed we can just increment or decrement a counter
        size_t count = 0;
        for (const auto& _resources : typeResources) {
            count += _resources.second.size();
        }
        return count;
    }

	void ResourceManager::prune() {
        for (auto& type_resource : typeResources) {
            auto& _resources = type_resource.second;
            auto resources_itr = _resources.begin();
            while (resources_itr != _resources.end()) {
                if (resources_itr->second.unique()) {
                    resources_itr = _resources.erase(resources_itr);
                } else {
                    ++resources_itr;
                }
            }
        }
    }

	void ResourceManager::purge() {
#if defined(DEBUG)
        std::vector<boost::weak_ptr<Resource>> _resources;
        for (auto& type_resource : type_resources) {
            for (auto& resource : type_resource.second) {
                _resources.push_back(resource.second);
            }
        }
#endif
        typeResources.clear();
    }
}
