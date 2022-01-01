#include "gpuBuffer.hpp"
#include "../gpu.hpp"

namespace Rendering::GPU::Buffers {
	GpuBuffer::GpuBuffer() {
        id = gpu.createBuffer();
    }

	GpuBuffer::~GpuBuffer() {
        gpu.destroyBuffer(id);
    }
}
