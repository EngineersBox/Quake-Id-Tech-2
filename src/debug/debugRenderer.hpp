#pragma once

#ifndef QUAKE_DEBUGRENDERER_HPP
#define QUAKE_DEBUGRENDERER_HPP

#include "../device/gpu/buffers/vertexBuffer.hpp"
#include "../device/gpu/buffers/indexBuffer.hpp"
#include "../device/gpu/shaders/shaderManager.hpp"
#include "../device/gpu/shaders/programs/basicShader.hpp"
#include "../scene/structure/rectangle.hpp"
#include "../scene/structure/line.hpp"
#include "../scene/structure/sphere.hpp"
#include "../resources/texture.hpp"
#include "../resources/resourceManager.hpp"
#include "../device/gpu/buffers/gpuBufferManager.hpp"

namespace Debug::Renderer {
    inline void renderLineLoop(const glm::mat4& world_matrix, const glm::mat4& view_projection_matrix, const std::vector<glm::vec3>& points, const glm::vec4& color) {
        static const size_t MAX_LINES = 1024;
        static const size_t VERTEX_COUNT = MAX_LINES + 1;
        static const size_t INDEX_COUNT = MAX_LINES;

        assert(points.size() <= MAX_LINES);

        typedef Device::GPU::Buffers::VertexBuffer<Device::GPU::Shaders::Programs::BasicShader::VertexType> VertexBufferType;
        typedef Device::GPU::Buffers::IndexBuffer<Device::GPU::IndexType<INDEX_COUNT>::Type> IndexBufferType;

        static boost::weak_ptr<VertexBufferType> vertexBuffer;
        static boost::weak_ptr<IndexBufferType> indexBuffer;

        static std::array<Device::GPU::Shaders::Programs::BasicShader::VertexType, VERTEX_COUNT> vertices;

        if (vertexBuffer.expired()) {
            vertexBuffer = Device::GPU::Buffers::gpuBuffers.make<VertexBufferType>();
            for (Device::GPU::Shaders::Programs::BasicShader::Vertex& vertex : vertices) {
                vertex.color = glm::vec4(1);
            }
        }

        for (size_t i = 0; i < points.size(); ++i) {
            vertices[i].location = points[i];
        }

        vertexBuffer.lock()->data(vertices.data(), points.size(), Device::GPU::Gpu::BufferUsage::STREAM_DRAW);

        if (indexBuffer.expired()) {
            std::array<IndexBufferType::IndexType, INDEX_COUNT> indices;
            for (unsigned short i = 0; i < INDEX_COUNT; ++i) {
                indices[i] = i;
            }

            indexBuffer = Device::GPU::Buffers::gpuBuffers.make<IndexBufferType>();
            indexBuffer.lock()->data(indices.data(), indices.size(), Device::GPU::Gpu::BufferUsage::STATIC_DRAW);
        }

        Device::GPU::gpu.buffers.push(Device::GPU::Gpu::BufferTarget::ARRAY, vertexBuffer.lock());
        Device::GPU::gpu.buffers.push(Device::GPU::Gpu::BufferTarget::ELEMENT_ARRAY, indexBuffer.lock());

        const boost::shared_ptr<Device::GPU::Shaders::Programs::BasicShader> gpuProgram = Device::GPU::Shaders::shaders.get<Device::GPU::Shaders::Programs::BasicShader>();
        Device::GPU::gpu.programs.push(gpuProgram);

        boost::shared_ptr<Resources::Texture> texture = Resources::resources.get<Resources::Texture>("white.png");
        Device::GPU::gpu.textures.bind(0, texture);

        Device::GPU::gpu.setUniform("world_matrix", world_matrix);
        Device::GPU::gpu.setUniform("view_projection_matrix", view_projection_matrix);
        Device::GPU::gpu.setUniform("color", color);
        Device::GPU::gpu.setUniform("diffuse_texture", 0);

        Device::GPU::gpu.drawElements(Device::GPU::Gpu::PrimitiveType::LINE_STRIP, points.size(), IndexBufferType::DATA_TYPE, 0);
        Device::GPU::gpu.textures.unbind(0);

        Device::GPU::gpu.programs.pop();
        Device::GPU::gpu.buffers.pop(Device::GPU::Gpu::BufferTarget::ARRAY);
        Device::GPU::gpu.buffers.pop(Device::GPU::Gpu::BufferTarget::ELEMENT_ARRAY);
    }

    template<typename T>
    void renderRectangle(const glm::mat4& world_matrix, const glm::mat4& view_projection_matrix, const Scene::Structure::Rectangle<T>& rectangle, const glm::vec4& color, bool is_filled = false) {
        typedef Device::GPU::Buffers::VertexBuffer<Device::GPU::Shaders::Programs::BasicShader::VertexType> VertexBufferType;
        typedef Device::GPU::Buffers::IndexBuffer<unsigned char> IndexBufferType;

        static boost::weak_ptr<VertexBufferType> vertexBuffer;
        static boost::weak_ptr<IndexBufferType> indexBuffer;

        if (vertexBuffer.expired()) {
            vertexBuffer = Device::GPU::Buffers::gpuBuffers.make<VertexBufferType>();
            std::initializer_list<Device::GPU::Shaders::Programs::BasicShader::Vertex> vertices = {
                    Device::GPU::Shaders::Programs::BasicShader::VertexType(glm::vec3(0, 0, 0), glm::vec4(1)),
                    Device::GPU::Shaders::Programs::BasicShader::VertexType(glm::vec3(1, 0, 0), glm::vec4(1)),
                    Device::GPU::Shaders::Programs::BasicShader::VertexType(glm::vec3(1, 1, 0), glm::vec4(1)),
                    Device::GPU::Shaders::Programs::BasicShader::VertexType(glm::vec3(0, 1, 0), glm::vec4(1))
            };
            vertexBuffer.lock()->data(vertices, Device::GPU::Gpu::BufferUsage::STATIC_DRAW);
        }

        if (indexBuffer.expired()) {
            indexBuffer = Device::GPU::Buffers::gpuBuffers.make<IndexBufferType>();
            indexBuffer.lock()->data({ 0, 1, 2, 3 }, Device::GPU::Gpu::BufferUsage::STATIC_DRAW);
        }

        Device::GPU::gpu.buffers.push(Device::GPU::Gpu::BufferTarget::ARRAY, vertexBuffer.lock());
        Device::GPU::gpu.buffers.push(Device::GPU::Gpu::BufferTarget::ELEMENT_ARRAY, indexBuffer.lock());

        const boost::shared_ptr<Device::GPU::Shaders::Programs::BasicShader> gpuProgram = Device::GPU::Shaders::shaders.get<Device::GPU::Shaders::Programs::BasicShader>();
        Device::GPU::gpu.programs.push(gpuProgram);

        auto texture = Resources::resources.get<Resources::Texture>("white.png");
        Device::GPU::gpu.textures.bind(0, texture);

        Device::GPU::gpu.setUniform("world_matrix", world_matrix * glm::translate(vec3(rectangle.x, rectangle.y, T(0))) * glm::scale(vec3(rectangle.width, rectangle.height, T(0))));
        Device::GPU::gpu.setUniform("view_projection_matrix", view_projection_matrix);
        Device::GPU::gpu.setUniform("color", color);

        Device::GPU::gpu.drawElements(is_filled ? Device::GPU::Gpu::PrimitiveType::TRIANGLE_FAN : Device::GPU::Gpu::PrimitiveType::LINE_LOOP, 4, IndexBufferType::DATA_TYPE, 0);
        Device::GPU::gpu.textures.unbind(0);

        Device::GPU::gpu.programs.pop();
        Device::GPU::gpu.buffers.pop(Device::GPU::Gpu::BufferTarget::ELEMENT_ARRAY);
        Device::GPU::gpu.buffers.pop(Device::GPU::Gpu::BufferTarget::ARRAY);
    }

    void renderAxes(const glm::mat4& world_matrix, const glm::mat4& view_projection_matrix);

    template<typename T>
    void renderAabb(const glm::mat4& world_matrix, const glm::mat4& view_projection_matrix, const Scene::Structure::AABB3<T>& aabb, const glm::vec4& color) {
        typedef Device::GPU::Buffers::VertexBuffer<Device::GPU::Shaders::Programs::BasicShader::VertexType> VertexBufferType;
        typedef Device::GPU::Buffers::IndexBuffer<unsigned char> IndexBufferType;
        typedef VertexBufferType::VertexType VertexType;

        static boost::weak_ptr<VertexBufferType> vertexBuffer;
        static boost::weak_ptr<IndexBufferType> indexBuffer;

        if (vertexBuffer.expired()) {
            vertexBuffer = Device::GPU::Buffers::gpuBuffers.make<VertexBufferType>();
            auto vertices = {
                    VertexType(glm::vec3(0, 0, 0), glm::vec4(1)),
                    VertexType(glm::vec3(1, 0, 0), glm::vec4(1)),
                    VertexType(glm::vec3(1, 1, 0), glm::vec4(1)),
                    VertexType(glm::vec3(0, 1, 0), glm::vec4(1)),
                    VertexType(glm::vec3(0, 0, 1), glm::vec4(1)),
                    VertexType(glm::vec3(1, 0, 1), glm::vec4(1)),
                    VertexType(glm::vec3(1, 1, 1), glm::vec4(1)),
                    VertexType(glm::vec3(0, 1, 1), glm::vec4(1))
            };
            vertexBuffer.lock()->data(vertices, Device::GPU::Gpu::BufferUsage::DYNAMIC_DRAW);
        }

        if (indexBuffer.expired()) {
            indexBuffer = Device::GPU::Buffers::gpuBuffers.make<IndexBufferType>();
            indexBuffer.lock()->data({ 0, 1, 1, 2, 2, 3, 3, 0, 0, 4, 1, 5, 2, 6, 3, 7, 4, 5, 5, 6, 6, 7, 7, 4 }, Device::GPU::Gpu::BufferUsage::STATIC_DRAW);
        }

        Device::GPU::gpu.buffers.push(Device::GPU::Gpu::BufferTarget::ARRAY, vertexBuffer.lock());
        Device::GPU::gpu.buffers.push(Device::GPU::Gpu::BufferTarget::ELEMENT_ARRAY, indexBuffer.lock());
        const boost::shared_ptr<Device::GPU::Shaders::Programs::BasicShader> gpuProgram = Device::GPU::Shaders::shaders.get<Device::GPU::Shaders::Programs::BasicShader>();
        Device::GPU::gpu.programs.push(gpuProgram);

        Device::GPU::gpu.setUniform("world_matrix", world_matrix * glm::translate(aabb.min) * glm::scale(aabb.size()));
        Device::GPU::gpu.setUniform("view_projection_matrix", view_projection_matrix);
        Device::GPU::gpu.setUniform("color", color);

        boost::shared_ptr<Resources::Texture> texture = Resources::resources.get<Resources::Texture>("white.png");
        Device::GPU::gpu.textures.bind(0, texture);
        Device::GPU::gpu.drawElements(Device::GPU::Gpu::PrimitiveType::LINES, 24, IndexBufferType::DATA_TYPE, 0);

        Device::GPU::gpu.textures.unbind(0);
        Device::GPU::gpu.programs.pop();
        Device::GPU::gpu.buffers.pop(Device::GPU::Gpu::BufferTarget::ELEMENT_ARRAY);
        Device::GPU::gpu.buffers.pop(Device::GPU::Gpu::BufferTarget::ARRAY);
    }

    template<typename T>
    void renderSphere(const glm::mat4& world_matrix, const glm::mat4& view_projection_matrix, const Scene::Structure::Sphere<T>& sphere, const glm::vec4& color) {
        typedef Device::GPU::Buffers::VertexBuffer<Device::GPU::Shaders::Programs::BasicShader::VertexType> VertexBufferType;
        typedef Device::GPU::Buffers::IndexBuffer<unsigned char> IndexBufferType;

        static boost::weak_ptr<VertexBufferType> vertexBuffer;
        static boost::weak_ptr<IndexBufferType> indexBuffer;

        const size_t SPHERE_SIDES = 32;

        if (vertexBuffer.expired()) {
            vertexBuffer = Device::GPU::Buffers::gpuBuffers.make<VertexBufferType>();
            std::vector<Device::GPU::Shaders::Programs::BasicShader::VertexType> vertices;
            vertices.reserve(SPHERE_SIDES);

            for (int i = 0; i < SPHERE_SIDES; ++i) {
                float sigma = static_cast<float>(i) / SPHERE_SIDES * glm::pi<float>() * 2;
                vertices.emplace_back(glm::vec3(glm::sin(sigma), glm::cos(sigma), 0), glm::vec4(0, 0, 1, 1));
            }

            for (int i = 0; i < SPHERE_SIDES; ++i) {
                float sigma = static_cast<float>(i) / SPHERE_SIDES * glm::pi<float>() * 2;
                vertices.emplace_back(glm::vec3(0, glm::sin(sigma), glm::cos(sigma)), glm::vec4(1, 0, 0, 1));
            }

            for (int i = 0; i < SPHERE_SIDES; ++i) {
                float sigma = static_cast<float>(i) / SPHERE_SIDES * glm::pi<float>() * 2;
                vertices.emplace_back(glm::vec3(glm::cos(sigma), 0, glm::sin(sigma)), glm::vec4(0, 1, 0, 1));
            }

            vertexBuffer.lock()->data(vertices, Device::GPU::Gpu::BufferUsage::DYNAMIC_DRAW);
        }

        if (indexBuffer.expired()) {
            indexBuffer = Device::GPU::Buffers::gpuBuffers.make<IndexBufferType>();
            std::vector<unsigned char> indices;
            indices.reserve(SPHERE_SIDES * 3);

            for (int i = 0; i < SPHERE_SIDES * 3; ++i) {
                indices.push_back(i);
            }

            indexBuffer.lock()->data(indices, Device::GPU::Gpu::BufferUsage::STATIC_DRAW);
        }

        Device::GPU::gpu.buffers.push(Device::GPU::Gpu::BufferTarget::ARRAY, vertexBuffer.lock());
        Device::GPU::gpu.buffers.push(Device::GPU::Gpu::BufferTarget::ELEMENT_ARRAY, indexBuffer.lock());

        const boost::shared_ptr<Device::GPU::Shaders::Programs::BasicShader> gpuProgram = Device::GPU::Shaders::shaders.get<Device::GPU::Shaders::Programs::BasicShader>();
        Device::GPU::gpu.programs.push(gpuProgram);

        Device::GPU::gpu.setUniform("world_matrix", world_matrix * glm::translate(sphere.origin) * glm::scale(vec3(sphere.radius)));
        Device::GPU::gpu.setUniform("view_projection_matrix", view_projection_matrix);
        Device::GPU::gpu.setUniform("color", color);

        boost::shared_ptr<Resources::Texture> texture = Resources::resources.get<Resources::Texture>("white.png");
        Device::GPU::gpu.textures.bind(0, texture);

        Device::GPU::gpu.drawElements(Device::GPU::Gpu::PrimitiveType::LINE_LOOP, SPHERE_SIDES, IndexBufferType::DATA_TYPE, SPHERE_SIDES * 0);
        Device::GPU::gpu.drawElements(Device::GPU::Gpu::PrimitiveType::LINE_LOOP, SPHERE_SIDES, IndexBufferType::DATA_TYPE, SPHERE_SIDES * 1);
        Device::GPU::gpu.drawElements(Device::GPU::Gpu::PrimitiveType::LINE_LOOP, SPHERE_SIDES, IndexBufferType::DATA_TYPE, SPHERE_SIDES * 2);

        Device::GPU::gpu.textures.unbind(0);
        Device::GPU::gpu.programs.pop();
        Device::GPU::gpu.buffers.pop(Device::GPU::Gpu::BufferTarget::ELEMENT_ARRAY);
        Device::GPU::gpu.buffers.pop(Device::GPU::Gpu::BufferTarget::ARRAY);
    }
}

#endif //QUAKE_DEBUGRENDERER_HPP
