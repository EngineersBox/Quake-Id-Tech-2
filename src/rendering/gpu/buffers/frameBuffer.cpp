#include "frameBuffer.hpp"
#include "../gpu.hpp"

namespace Rendering::GPU::Buffers {
	FrameBuffer::FrameBuffer(GpuFrameBufferType type, const GpuFrameBufferSizeType& size) :
        type(type),
        size(size) {
        id = gpu.createFrameBuffer(type, size, colorTexture, depthStencilTexture, depthTexture);
    }

	FrameBuffer::~FrameBuffer() {
        gpu.destroyFrameBuffer(id);
    }

	void FrameBuffer::on_bind() const {
        //NOTE: setting these flags on binding could have unforseen consequences
        Gpu::ColorStateManager::ColorState gpu_color_state = gpu.color.getState();
        Gpu::Depth::State gpu_depth_state = gpu.depth.getState();

        bool maskValue = (static_cast<GpuFrameBufferTypeFlagsType>(type)& GPU_FRAME_BUFFER_TYPE_FLAG_COLOR) == GPU_FRAME_BUFFER_TYPE_FLAG_COLOR;
        gpu_color_state.mask.r = maskValue;
        gpu_color_state.mask.g = maskValue;
        gpu_color_state.mask.b = maskValue;
        gpu_color_state.mask.a = maskValue;

		if ((static_cast<GpuFrameBufferTypeFlagsType>(type) & GPU_FRAME_BUFFER_TYPE_FLAG_DEPTH) == GPU_FRAME_BUFFER_TYPE_FLAG_DEPTH) {
            gpu_depth_state.shouldWriteMask = true;
        } else {
            gpu_depth_state.shouldWriteMask = false;
        }

        gpu.color.pushState(gpu_color_state);
        gpu.depth.pushState(gpu_depth_state);

        //TODO: stencil mask
    }

	void FrameBuffer::setSize(const GpuFrameBufferSizeType& _size) {
        if (_size == getSize()) return;

        if (colorTexture != nullptr) colorTexture->set_size(_size);
        if (depthTexture != nullptr) depthTexture->set_size(_size);
        if (depthStencilTexture != nullptr) depthStencilTexture->set_size(_size);

        this->size = _size;
    }
}
