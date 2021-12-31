#include "shader.hpp"
#include "../gpu.hpp"

namespace Rendering::GPU::Shaders {
	Shader::Shader(const std::string& vertex_shader_source, const std::string& fragment_shader_source) {
        id = gpu.create_program(vertex_shader_source, fragment_shader_source);
    }

	Shader::~Shader() {
        gpu.destroy_program(id);
    }
}
