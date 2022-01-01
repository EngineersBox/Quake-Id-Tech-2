#pragma once

#include <boost/enable_shared_from_this.hpp>

#include "../gpu.hpp"

namespace Rendering::GPU::Shaders {
	struct Shader : boost::enable_shared_from_this<Shader> {
		virtual ~Shader();

        virtual void onBind() = 0;
        virtual void onUnbind() = 0;

        GpuId getId() const { return id; }

    protected:
		Shader(const std::string& vertex_shader_source, const std::string& fragment_shader_source);

    private:
		GpuId id;

		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;
    };

    template<typename T, typename Enable = void>
    struct IsGpuProgram : std::false_type {};

    template<typename T>
	struct IsGpuProgram<T, typename std::enable_if<std::is_base_of<Shader, T>::value>::type> : std::true_type {};
}