#include "gpuBufferManager.hpp"

namespace Device::GPU::Buffers {
	GpuBufferManager gpuBuffers;
	void GpuBufferManager::purge() { buffers.clear(); }
}