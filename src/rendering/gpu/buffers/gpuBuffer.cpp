#include "gpuBuffer.hpp"
#include "../gpu.hpp"

namespace Rendering::GPU::Buffers {
	GpuBuffer::GpuBuffer() {
        id = gpu.create_buffer();
    }

	GpuBuffer::~GpuBuffer() {
        gpu.destroy_buffer(id);
    }
}
