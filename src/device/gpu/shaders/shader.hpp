#pragma once

#include <boost/enable_shared_from_this.hpp>
#include <concepts>

#include "../gpu.hpp"

namespace Device::GPU::Shaders {
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

    template<typename T>
    concept IsShader = std::derived_from<T, Shader>;
}