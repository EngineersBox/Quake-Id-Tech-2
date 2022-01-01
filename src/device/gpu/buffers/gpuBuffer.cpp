#include "gpuBuffer.hpp"
#include "../gpu.hpp"

namespace Device::GPU::Buffers {
	GpuBuffer::GpuBuffer() {
        id = gpu.createBuffer();
    }

	GpuBuffer::~GpuBuffer() {
        gpu.destroyBuffer(id);
    }
}
