#pragma once

#include <map>
#include <boost/make_shared.hpp>
#include <concepts>

#include "gpuBuffer.hpp"

namespace Device::GPU::Buffers {
    struct GpuBufferManager {
        template<typename T> requires IsGpuBuffer<T>
        boost::weak_ptr<T> make() {
            boost::shared_ptr<T> gpuBuffer = boost::make_shared<T>();
            auto buffersIter = buffers.insert(buffers.begin(), std::make_pair(gpuBuffer->getId(), gpuBuffer));
			return boost::static_pointer_cast<T, GpuBuffer>(buffersIter->second);
        }

        void purge();

    private:
		std::map<GpuId, boost::shared_ptr<GpuBuffer>> buffers;
    };

	extern GpuBufferManager gpuBuffers;
}