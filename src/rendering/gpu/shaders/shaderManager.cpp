#include "shaderManager.hpp"

namespace Rendering::GPU::Shaders {
	ShaderManager shaders;

	void ShaderManager::purge() {
#if defined(DEBUG)
        for (auto& _shader : shaders) {
            assert(_shader.second.unique());
        }
#endif
        shaders.clear();
    }
}