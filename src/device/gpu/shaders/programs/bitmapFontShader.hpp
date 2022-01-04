#pragma once

#ifndef QUAKE_BITMAPFONTSHADER_H
#define QUAKE_BITMAPFONTSHADER_H

#include <glm/glm.hpp>

#include "../shader.hpp"

namespace Device::GPU::Shaders::Programs {
    struct BitmapFontShader : Shader {
        struct Vertex {
            Vertex() = default;
            Vertex(glm::vec2 location, glm::vec2 texcoord) {
                this->location = location;
                this->texcoord = texcoord;
            }

            glm::vec2 location;
            glm::vec2 texcoord;
        };

        typedef Vertex VertexType;

        BitmapFontShader() :
                Shader(R"(#version 400

precision lowp float;

uniform mat4 world_matrix;
uniform mat4 view_projection_matrix;
uniform float line_height;
uniform float base;
uniform vec4 color_top;
uniform vec4 color_bottom;
uniform int should_invert_rgb;

in vec2 location;
in vec2 texcoord;

out vec2 out_texcoord;
out vec4 out_color;

void main()
{
    gl_Position = (view_projection_matrix) * (world_matrix * vec4(location, 0, 1));

    out_texcoord = texcoord;

    float t = (location.y + (line_height - base)) / line_height;
    out_color = mix(color_bottom, color_top, t);

    //TODO: convert to color modifier subroutine
    if (should_invert_rgb == 1)
    {
        out_color.rgb = vec3(1) - out_color.rgb;
    }
})", R"(#version 400

uniform sampler2D diffuse_texture;

in vec2 out_texcoord;
in vec4 out_color;

void main()
{
    gl_FragColor = texture2D(diffuse_texture, out_texcoord) * out_color;
})")
        {
            this->locationLocation = gpu.getAttributeLocation(getId(), "location");
            this->texcoordLocation = gpu.getAttributeLocation(getId(), "texcoord");
        }

        void onBind() override {
            gpu.enableVertexAttributeArray(this->locationLocation);
            gpu.enableVertexAttributeArray(this->texcoordLocation);
            gpu.setVertexAttribPointer(this->locationLocation, sizeof(glm::vec2) / sizeof(glm::vec2::value_type), GpuDataType<glm::vec2::value_type>::VALUE, false, sizeof(VertexType), reinterpret_cast<void*>(offsetof(VertexType, location)));
            gpu.setVertexAttribPointer(this->texcoordLocation, sizeof(glm::vec2) / sizeof(glm::vec2::value_type), GpuDataType<glm::vec2::value_type>::VALUE, false, sizeof(VertexType), reinterpret_cast<void*>(offsetof(VertexType, texcoord)));
        }

        void onUnbind() override {
            gpu.disableVertexAttributeArray(this->locationLocation);
            gpu.disableVertexAttributeArray(this->texcoordLocation);
        }


    private:
        GpuLocation locationLocation;
        GpuLocation texcoordLocation;
    };
}

#endif //QUAKE_BITMAPFONTSHADER_H
