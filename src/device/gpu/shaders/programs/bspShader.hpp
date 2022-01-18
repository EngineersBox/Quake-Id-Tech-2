#pragma once

#include "../shader.hpp"
#include <glm/glm.hpp>

namespace Device::GPU::Shaders::Programs {
    struct BSPShader : Device::GPU::Shaders::Shader {
        struct Vertex {
            Vertex() = default;
            Vertex(glm::vec3 location, glm::vec2 diffuse_texcoord, glm::vec2 lightmap_texcoord) {
                this->location = location;
                this->diffuseTexcoord = diffuse_texcoord;
                this->lightmapTexcoord = lightmap_texcoord;
            }

            glm::vec3 location;
            glm::vec2 diffuseTexcoord;
            glm::vec2 lightmapTexcoord;
        };

        typedef Vertex VertexType;

        BSPShader() : Shader(
            Resources::IO::readFile("shaders/bsp/bsp.vert"),
            Resources::IO::readFile("shaders/bsp/bsp.frag")
        ) {
            locationLocation = Device::GPU::gpu.getAttributeLocation(getId(), "location");
            diffuseTexcoordLocation = Device::GPU::gpu.getAttributeLocation(getId(), "diffuse_texcoord");
            lightmapTexcoordLocation = Device::GPU::gpu.getAttributeLocation(getId(), "lightmap_texcoord");
        }

        void onBind() override {
            Device::GPU::gpu.enableVertexAttributeArray(locationLocation);
            Device::GPU::gpu.enableVertexAttributeArray(diffuseTexcoordLocation);
            Device::GPU::gpu.enableVertexAttributeArray(lightmapTexcoordLocation);
            Device::GPU::gpu.setVertexAttribPointer(
                locationLocation, sizeof(glm::vec3) / sizeof(glm::vec3::value_type),
                Device::GPU::GpuDataType<glm::vec3::value_type>::VALUE, false, sizeof(VertexType),
                reinterpret_cast<void *>(offsetof(VertexType, location))
            );
            Device::GPU::gpu.setVertexAttribPointer(
                diffuseTexcoordLocation,
                sizeof(glm::vec2) / sizeof(glm::vec2::value_type),
                Device::GPU::GpuDataType<glm::vec2::value_type>::VALUE, false, sizeof(VertexType),
                reinterpret_cast<void *>(offsetof(VertexType, diffuseTexcoord))
            );
            Device::GPU::gpu.setVertexAttribPointer(
                lightmapTexcoordLocation,
                sizeof(glm::vec2) / sizeof(glm::vec2::value_type),
                Device::GPU::GpuDataType<glm::vec2::value_type>::VALUE, false, sizeof(VertexType),
                reinterpret_cast<void *>(offsetof(VertexType, lightmapTexcoord))
            );
        }

        void onUnbind() override {
            Device::GPU::gpu.disableVertexAttributeArray(locationLocation);
            Device::GPU::gpu.disableVertexAttributeArray(diffuseTexcoordLocation);
            Device::GPU::gpu.disableVertexAttributeArray(lightmapTexcoordLocation);
        }


    private:
        Device::GPU::GpuLocation locationLocation;
        Device::GPU::GpuLocation diffuseTexcoordLocation;
        Device::GPU::GpuLocation lightmapTexcoordLocation;
    };
}