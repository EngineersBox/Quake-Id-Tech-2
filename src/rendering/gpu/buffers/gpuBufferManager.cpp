#include "gpuBufferManager.hpp"

namespace Rendering::GPU::Buffers {
	GpuBufferManager gpuBuffers;
	void GpuBufferManager::purge() { buffers.clear(); }
}