#pragma once

#include <map>
#include <boost/make_shared.hpp>

#include "gpuBuffer.hpp"

namespace Rendering::GPU::Buffers {
    struct GpuBufferManager {
        template<typename T, std::enable_if_t<IsGpuBuffer<T>::value, bool> = true>
        boost::weak_ptr<T> make() {
            boost::shared_ptr<T> gpuBuffer = boost::make_shared<T>();
            auto buffersIter = buffers.insert(buffers.begin(), std::make_pair(gpuBuffer->get_id(), gpuBuffer));
			return boost::static_pointer_cast<T, GpuBuffer>(buffersIter->second);
        }

        void purge();

    private:
		std::map<GpuId, boost::shared_ptr<GpuBuffer>> buffers;
    };

	extern GpuBufferManager gpuBuffers;
}