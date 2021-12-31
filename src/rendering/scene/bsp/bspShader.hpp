#pragma once

#include "../../gpu/shaders/shader.hpp"
#include <glm/glm.hpp>

namespace Rendering::Scene {
    struct BSPShader : GPU::Shaders::Shader {
        struct Vertex {
            Vertex() = default;
            Vertex(glm::vec3 location, glm::vec2 diffuse_texcoord, glm::vec2 lightmap_texcoord) {
                this->location = location;
                this->diffuse_texcoord = diffuse_texcoord;
                this->lightmap_texcoord = lightmap_texcoord;
            }

            glm::vec3 location;
            glm::vec2 diffuse_texcoord;
            glm::vec2 lightmap_texcoord;
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
            location_location = GPU::gpu.get_attribute_location(get_id(), "location");
            diffuse_texcoord_location = GPU::gpu.get_attribute_location(get_id(), "diffuse_texcoord");
            lightmap_texcoord_location = GPU::gpu.get_attribute_location(get_id(), "lightmap_texcoord");
        }

        void on_bind() override {
            GPU::gpu.enable_vertex_attribute_array(location_location);
            GPU::gpu.enable_vertex_attribute_array(diffuse_texcoord_location);
            GPU::gpu.enable_vertex_attribute_array(lightmap_texcoord_location);
            GPU::gpu.set_vertex_attrib_pointer(location_location, sizeof(glm::vec3) / sizeof(glm::vec3::value_type), GPU::GpuDataType<glm::vec3::value_type>::VALUE, false, sizeof(VertexType), reinterpret_cast<void*>(offsetof(VertexType, location)));
            GPU::gpu.set_vertex_attrib_pointer(diffuse_texcoord_location, sizeof(glm::vec2) / sizeof(glm::vec2::value_type), GPU::GpuDataType<glm::vec2::value_type>::VALUE, false, sizeof(VertexType), reinterpret_cast<void*>(offsetof(VertexType, diffuse_texcoord)));
            GPU::gpu.set_vertex_attrib_pointer(lightmap_texcoord_location, sizeof(glm::vec2) / sizeof(glm::vec2::value_type), GPU::GpuDataType<glm::vec2::value_type>::VALUE, false, sizeof(VertexType), reinterpret_cast<void*>(offsetof(VertexType, lightmap_texcoord)));
        }

        void on_unbind() override {
            GPU::gpu.disable_vertex_attribute_array(location_location);
            GPU::gpu.disable_vertex_attribute_array(diffuse_texcoord_location);
            GPU::gpu.disable_vertex_attribute_array(lightmap_texcoord_location);
        }


    private:
        GPU::GpuLocation location_location;
        GPU::GpuLocation diffuse_texcoord_location;
        GPU::GpuLocation lightmap_texcoord_location;
    };
}