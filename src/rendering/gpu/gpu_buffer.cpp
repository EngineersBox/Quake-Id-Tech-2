#include "gpu_buffer.hpp"
#include "gpu.hpp"

namespace Rendering::GPU {
	GpuBuffer::GpuBuffer() {
        id = gpu.create_buffer();
    }

	GpuBuffer::~GpuBuffer() {
        gpu.destroy_buffer(id);
    }
}
