#include "shader.hpp"
#include "../gpu.hpp"

namespace Device::GPU::Shaders {
	Shader::Shader(const std::string& vertex_shader_source, const std::string& fragment_shader_source) {
        id = gpu.createProgram(vertex_shader_source, fragment_shader_source);
    }

	Shader::~Shader() {
        gpu.destroyProgram(id);
    }
}
