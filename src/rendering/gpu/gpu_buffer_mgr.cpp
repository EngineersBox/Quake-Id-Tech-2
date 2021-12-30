#include "gpu_buffer_mgr.hpp"

namespace Rendering::GPU {
	GpuBufferManager gpu_buffers;
	void GpuBufferManager::purge() { buffers.clear(); }
}