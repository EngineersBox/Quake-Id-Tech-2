#pragma once

#include <typeindex>
#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include "shader.hpp"

namespace Rendering::GPU::Shaders {
    struct ShaderManager {
		template<typename T, std::enable_if_t<IsGpuProgram<T>::value, bool> = true>
        boost::shared_ptr<T> make() {
            static const std::type_index TYPE_INDEX = typeid(T);
            const boost::shared_ptr<T> _shader = boost::make_shared<T>();
            shaders.insert(std::make_pair(TYPE_INDEX, _shader));
            return _shader;
        }

        template<typename T, std::enable_if_t<IsGpuProgram<T>::value, bool> = true>
        boost::shared_ptr<T> get() {
            static const std::type_index TYPE_INDEX = typeid(T);
            const auto shader_iter = shaders.find(TYPE_INDEX);
            if (shader_iter == shaders.end()) {
                throw std::out_of_range("gpu program does not exist");
            }
            return boost::static_pointer_cast<T, Shader>(shader_iter->second);
        }

        void purge();

    private:
        std::map<std::type_index, boost::shared_ptr<Shader>> shaders;
    };

	extern ShaderManager shaders;
}
