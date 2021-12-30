#pragma once

#include <map>
#include <boost/make_shared.hpp>

#include "gpu_buffer.hpp"

namespace Rendering::GPU {
    struct GpuBufferManager {
        template<typename T = std::enable_if<IsGpuBuffer<T>::value, T>::type>
        boost::weak_ptr<T> make() {
            boost::shared_ptr<T> gpu_buffer = boost::make_shared<T>();
            auto buffers_itr = buffers.insert(buffers.begin(), std::make_pair(gpu_buffer->get_id(), gpu_buffer));
			return boost::static_pointer_cast<T, GpuBuffer>(buffers_itr->second);
        }

        void purge();

    private:
		std::map<GpuId, boost::shared_ptr<GpuBuffer>> buffers;
    };

	extern GpuBufferManager gpu_buffers;
}