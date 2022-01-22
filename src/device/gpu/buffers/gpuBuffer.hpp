#pragma once

#include <boost/enable_shared_from_this.hpp>
#include <concepts>
#include "../gpuDefs.hpp"

namespace Device::GPU::Buffers {
	struct GpuBuffer : boost::enable_shared_from_this<GpuBuffer> {
        GpuId getId() const { return id; }
		virtual ~GpuBuffer();
    protected:
		GpuBuffer();
    private:
		GpuId id;
    };

    template<typename T>
    concept IsGpuBuffer = std::derived_from<T, GpuBuffer>;
}
