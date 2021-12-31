#pragma once

#include <boost/enable_shared_from_this.hpp>
#include "../gpuDefs.hpp"

namespace Rendering::GPU::Buffers {
	struct GpuBuffer : boost::enable_shared_from_this<GpuBuffer> {
        GpuId get_id() const { return id; }
		virtual ~GpuBuffer();
    protected:
		GpuBuffer();
    private:
		GpuId id;
    };

    template<typename T, typename Enable = void>
    struct IsGpuBuffer : std::false_type {};

    template<typename T>
	struct IsGpuBuffer<T, typename std::enable_if<std::is_base_of<GpuBuffer, T>::value>::type> : std::true_type {};
}
