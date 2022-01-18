#pragma once

#ifndef QUAKE_BLURHORIZONTALSHADER_HPP
#define QUAKE_BLURHORIZONTALSHADER_HPP

#include <glm/glm.hpp>

#include "../../../../resources/io/fileReader.hpp"
#include "../shader.hpp"

namespace Device::GPU::Shaders::Programs {
    struct BlurHorizontalShader : Shader {
        struct Vertex {
            Vertex() = default;
            Vertex(glm::vec3 location, glm::vec2 texcoord) {
                this->location = location;
                this->texcoord = texcoord;
            }

            glm::vec3 location;
            glm::vec2 texcoord;
        };

        typedef Vertex VertexType;

        BlurHorizontalShader() : Shader(
            Resources::IO::readFile("shaders/blurHorizontal/blurHorizontal.vert"),
            Resources::IO::readFile("shaders/blurHorizontal/blurHorizontal.frag")
        ) {
            locationLocation = gpu.getAttributeLocation(getId(), "location");
            texcoordLocation = gpu.getAttributeLocation(getId(), "texcoord");
        }

        void onBind() override {
            gpu.enableVertexAttributeArray(locationLocation);
            gpu.enableVertexAttributeArray(texcoordLocation);
            gpu.setVertexAttribPointer(locationLocation, sizeof(glm::vec3) / sizeof(glm::vec3::value_type), GpuDataType<glm::vec3::value_type>::VALUE, false, sizeof(VertexType), reinterpret_cast<void*>(offsetof(VertexType, location)));
            gpu.setVertexAttribPointer(texcoordLocation, sizeof(glm::vec2) / sizeof(glm::vec2::value_type), GpuDataType<glm::vec2::value_type>::VALUE, false, sizeof(VertexType), reinterpret_cast<void*>(offsetof(VertexType, texcoord)));
        }

        void onUnbind() override {
            gpu.disableVertexAttributeArray(locationLocation);
            gpu.disableVertexAttributeArray(texcoordLocation);
        }

    private:
        GpuLocation locationLocation;
        GpuLocation texcoordLocation;
    };
}

#endif //QUAKE_BLURHORIZONTALSHADER_HPP
