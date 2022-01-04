#pragma once

#ifndef QUAKE_BASICSHADER_HPP
#define QUAKE_BASICSHADER_HPP

#include "../shader.hpp"
#include <glm/glm.hpp>

namespace Device::GPU::Shaders::Programs {
    struct BasicShader : Shader {
        struct Vertex {
            Vertex() = default;
            Vertex(glm::vec3 location, glm::vec4 color)
            {
                this->location = location;
                this->color = color;
            }

            glm::vec3 location;
            glm::vec4 color;
        };

        typedef Vertex VertexType;

        BasicShader() :
                Shader(R"(#version 400

uniform mat4 world_matrix;
uniform mat4 view_projection_matrix;
uniform float texture_scale;

in vec3 location;
in vec4 color;

out vec4 out_color;
out vec2 out_texcoord;

void main()
{
    gl_Position = view_projection_matrix * (world_matrix * vec4(location, 1));

    out_texcoord.x = location.x * texture_scale;
    out_texcoord.y = location.z * texture_scale;

    out_color = color;
}
)", R"(#version 400

uniform sampler2D diffuse_texture;
uniform vec4 color;

in vec4 out_color;
in vec2 out_texcoord;

out vec4 fragment;

void main()
{
    fragment = texture2D(diffuse_texture, out_texcoord) * out_color * color;
}
)")
        {
            this->locationLocation = gpu.getAttributeLocation(getId(), "location");
            this->colorLocation = gpu.getAttributeLocation(getId(), "color");
        }

        void onBind() override {
            gpu.enableVertexAttributeArray(this->locationLocation);
            gpu.enableVertexAttributeArray(this->colorLocation);
            gpu.setVertexAttribPointer(this->locationLocation, sizeof(glm::vec3) / sizeof(glm::vec3::value_type), GpuDataType<glm::vec3::value_type>::VALUE, false, sizeof(VertexType), reinterpret_cast<void*>(offsetof(VertexType, location)));
            gpu.setVertexAttribPointer(this->colorLocation, sizeof(glm::vec4) / sizeof(glm::vec4::value_type), GpuDataType<glm::vec4::value_type>::VALUE, false, sizeof(VertexType), reinterpret_cast<void*>(offsetof(VertexType, color)));
        }

        void onUnbind() override {
            gpu.disableVertexAttributeArray(this->locationLocation);
            gpu.disableVertexAttributeArray(this->colorLocation);
        }


    private:
        GpuLocation locationLocation;
        GpuLocation colorLocation;
    };
}

#endif //QUAKE_BASICSHADER_HPP
