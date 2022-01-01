#pragma once

#include "../../gpu/shaders/shader.hpp"
#include <glm/glm.hpp>

namespace Rendering::Scene {
    struct BSPShader : GPU::Shaders::Shader {
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

        BSPShader() :
                // TODO: Read frag + vert from filesystem and cache before passing to shader instance
                Shader(R"(#version 400

precision lowp float;

uniform mat4 world_matrix;
uniform mat4 view_projection_matrix;
uniform sampler2D diffuse_texture;
uniform sampler2D lightmap_texture;

in vec3 location;
in vec2 diffuse_texcoord;
in vec2 lightmap_texcoord;

out vec2 out_diffuse_texcoord;
out vec2 out_lightmap_texcoord;

void main() {
    gl_Position = view_projection_matrix * (world_matrix * vec4(location, 1));

    //out_normal = normal;
    out_diffuse_texcoord = diffuse_texcoord;
    out_lightmap_texcoord = lightmap_texcoord;
})", R"(#version 400

uniform sampler2D diffuse_texture;
uniform sampler2D lightmap_texture;
uniform float lightmap_gamma;
uniform float alpha;
uniform bool should_test_alpha;

in vec3 out_normal;
in vec2 out_diffuse_texcoord;
in vec2 out_lightmap_texcoord;

out vec4 fragment;

void main() {
    vec4 lightmap_term = texture(lightmap_texture, out_lightmap_texcoord);
    lightmap_term.xyz *= lightmap_gamma;

    vec4 diffuse_term = texture(diffuse_texture, out_diffuse_texcoord);
    fragment = diffuse_term * lightmap_term;
    fragment.a *= alpha;

    if (should_test_alpha && fragment.a <= 0.25) {
        discard;
    }
})") {
            locationLocation = GPU::gpu.getAttributeLocation(getId(), "location");
            diffuseTexcoordLocation = GPU::gpu.getAttributeLocation(getId(), "diffuse_texcoord");
            lightmapTexcoordLocation = GPU::gpu.getAttributeLocation(getId(), "lightmap_texcoord");
        }

        void onBind() override {
            GPU::gpu.enableVertexAttributeArray(locationLocation);
            GPU::gpu.enableVertexAttributeArray(diffuseTexcoordLocation);
            GPU::gpu.enableVertexAttributeArray(lightmapTexcoordLocation);
            GPU::gpu.setVertexAttribPointer(
                locationLocation, sizeof(glm::vec3) / sizeof(glm::vec3::value_type),
                GPU::GpuDataType<glm::vec3::value_type>::VALUE, false, sizeof(VertexType),
                reinterpret_cast<void *>(offsetof(VertexType, location))
            );
            GPU::gpu.setVertexAttribPointer(
                diffuseTexcoordLocation,
                sizeof(glm::vec2) / sizeof(glm::vec2::value_type),
                GPU::GpuDataType<glm::vec2::value_type>::VALUE, false, sizeof(VertexType),
                reinterpret_cast<void *>(offsetof(VertexType, diffuseTexcoord))
            );
            GPU::gpu.setVertexAttribPointer(
                lightmapTexcoordLocation,
                sizeof(glm::vec2) / sizeof(glm::vec2::value_type),
                GPU::GpuDataType<glm::vec2::value_type>::VALUE, false, sizeof(VertexType),
                reinterpret_cast<void *>(offsetof(VertexType, lightmapTexcoord))
            );
        }

        void onUnbind() override {
            GPU::gpu.disableVertexAttributeArray(locationLocation);
            GPU::gpu.disableVertexAttributeArray(diffuseTexcoordLocation);
            GPU::gpu.disableVertexAttributeArray(lightmapTexcoordLocation);
        }


    private:
        GPU::GpuLocation locationLocation;
        GPU::GpuLocation diffuseTexcoordLocation;
        GPU::GpuLocation lightmapTexcoordLocation;
    };
}