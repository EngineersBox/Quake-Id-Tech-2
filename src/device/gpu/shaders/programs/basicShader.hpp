#pragma once

#ifndef QUAKE_BASICSHADER_HPP
#define QUAKE_BASICSHADER_HPP

#include "../shader.hpp"
#include <glm/glm.hpp>
#include "../../../../resources/io/fileReader.hpp"

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

        BasicShader() : Shader(
            Resources::IO::readFile("shaders/basic/basic.vert"),
            Resources::IO::readFile("shaders/basic/basic.frag")
        ) {
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
