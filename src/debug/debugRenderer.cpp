#include "debugRenderer.hpp"

namespace Debug::Renderer {
    void renderAxes(const glm::mat4& world_matrix, const glm::mat4& view_projection_matrix) {
        typedef Device::GPU::Buffers::VertexBuffer<Device::GPU::Shaders::Programs::BasicShader::VertexType> VertexBufferType;
        typedef Device::GPU::Buffers::IndexBuffer<unsigned char> IndexBufferType;
        typedef VertexBufferType::VertexType VertexType;

        static boost::weak_ptr<VertexBufferType> vertexBuffer;
        static boost::weak_ptr<IndexBufferType> indexBuffer;

        if (vertexBuffer.expired()) {
            vertexBuffer = Device::GPU::Buffers::gpuBuffers.make<VertexBufferType>();
            std::initializer_list<Device::GPU::Shaders::Programs::BasicShader::Vertex> vertices = {
                    VertexType(glm::vec3(0, 0, 0), glm::vec4(1, 0, 0, 1)),
                    VertexType(glm::vec3(1, 0, 0), glm::vec4(1, 0, 0, 1)),
                    VertexType(glm::vec3(0, 0, 0), glm::vec4(0, 1, 0, 1)),
                    VertexType(glm::vec3(0, 1, 0), glm::vec4(0, 1, 0, 1)),
                    VertexType(glm::vec3(0, 0, 0), glm::vec4(0, 0, 1, 1)),
                    VertexType(glm::vec3(0, 0, 1), glm::vec4(0, 0, 1, 1))
            };
            vertexBuffer.lock()->data(vertices, Device::GPU::Gpu::BufferUsage::STATIC_DRAW);
        }

        if (indexBuffer.expired()) {
            indexBuffer = Device::GPU::Buffers::gpuBuffers.make<IndexBufferType>();
            indexBuffer.lock()->data({ 0, 1, 2, 3, 4, 5 }, Device::GPU::Gpu::BufferUsage::STATIC_DRAW);
        }

        Device::GPU::gpu.buffers.push(Device::GPU::Gpu::BufferTarget::ARRAY, vertexBuffer.lock());
        Device::GPU::gpu.buffers.push(Device::GPU::Gpu::BufferTarget::ELEMENT_ARRAY, indexBuffer.lock());

        const boost::shared_ptr<Device::GPU::Shaders::Programs::BasicShader> gpuProgram = Device::GPU::Shaders::shaders.get<Device::GPU::Shaders::Programs::BasicShader>();
        Device::GPU::gpu.programs.push(gpuProgram);

        Device::GPU::gpu.setUniform("world_matrix", world_matrix);
        Device::GPU::gpu.setUniform("view_projection_matrix", view_projection_matrix);
        Device::GPU::gpu.setUniform("color", glm::vec4(1));

        boost::shared_ptr<Resources::Texture> texture = Resources::resources.get<Resources::Texture>("white.png");
        Device::GPU::gpu.textures.bind(0, texture);

        Device::GPU::gpu.drawElements(Device::GPU::Gpu::PrimitiveType::LINES, 6, IndexBufferType::DATA_TYPE, 0);
        Device::GPU::gpu.textures.unbind(0);

        Device::GPU::gpu.programs.pop();
        Device::GPU::gpu.buffers.pop(Device::GPU::Gpu::BufferTarget::ELEMENT_ARRAY);
        Device::GPU::gpu.buffers.pop(Device::GPU::Gpu::BufferTarget::ARRAY);
    }
}