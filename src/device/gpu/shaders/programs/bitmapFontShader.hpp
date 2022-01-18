#pragma once

#ifndef QUAKE_BITMAPFONTSHADER_H
#define QUAKE_BITMAPFONTSHADER_H

#include <glm/glm.hpp>

#include "../../../../resources/io/fileReader.hpp"
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

        BitmapFontShader() : Shader(
            Resources::IO::readFile("shaders/bitmapFont/bitmapFont.vert"),
            Resources::IO::readFile("shaders/bitmapFont/bitmapFont.frag")
        ) {
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
