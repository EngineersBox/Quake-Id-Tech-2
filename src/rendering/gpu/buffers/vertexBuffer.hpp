#pragma once

#include <array>
#include <vector>

#include "gpuBuffer.hpp"
#include "../gpu.hpp"

namespace Rendering::GPU::Buffers {
    template<typename Vertex>
	struct VertexBuffer : GpuBuffer {
        typedef Vertex VertexType;
        
		static const unsigned long VERTEX_SIZE = sizeof(VertexType);

		VertexBuffer() = default;

		void data(const VertexType* vertices, std::size_t count, Gpu::BufferUsage usage) {
            gpu.buffers.push(Gpu::BufferTarget::ARRAY, shared_from_this());
            gpu.buffers.data(Gpu::BufferTarget::ARRAY, static_cast<const void*>(vertices), VERTEX_SIZE * count, usage);
            gpu.buffers.pop(Gpu::BufferTarget::ARRAY);
        }

		void data(std::initializer_list<VertexType>& vertices, Gpu::BufferUsage usage) {
            data(vertices.begin(), vertices.size(), usage);
        }

		void data(std::vector<VertexType>& vertices, Gpu::BufferUsage usage) {
            data(vertices.data(), vertices.size(), usage);
        }

    private:
		VertexBuffer(const VertexBuffer&) = delete;
		VertexBuffer& operator=(const VertexBuffer&) = delete;
    };
}
