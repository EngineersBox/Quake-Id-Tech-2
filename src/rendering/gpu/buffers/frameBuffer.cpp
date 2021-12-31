#include "frameBuffer.hpp"
#include "../gpu.hpp"

namespace Rendering::GPU::Buffers {
	FrameBuffer::FrameBuffer(GpuFrameBufferType type, const GpuFrameBufferSizeType& size) :
        type(type),
        size(size) {
        id = gpu.create_frame_buffer(type, size, color_texture, depth_stencil_texture, depth_texture);
    }

	FrameBuffer::~FrameBuffer() {
        gpu.destroy_frame_buffer(id);
    }

	void FrameBuffer::on_bind() const {
        //NOTE: setting these flags on binding could have unforseen consequences
        Gpu::ColorStateManager::ColorState gpu_color_state = gpu.color.get_state();
        Gpu::depth::state gpu_depth_state = gpu.depth.get_state();

        bool maskValue = (static_cast<GpuFrameBufferTypeFlagsType>(type)& GPU_FRAME_BUFFER_TYPE_FLAG_COLOR) == GPU_FRAME_BUFFER_TYPE_FLAG_COLOR;
        gpu_color_state.mask.r = maskValue;
        gpu_color_state.mask.g = maskValue;
        gpu_color_state.mask.b = maskValue;
        gpu_color_state.mask.a = maskValue;

		if ((static_cast<GpuFrameBufferTypeFlagsType>(type) & GPU_FRAME_BUFFER_TYPE_FLAG_DEPTH) == GPU_FRAME_BUFFER_TYPE_FLAG_DEPTH) {
            gpu_depth_state.should_write_mask = true;
        } else {
            gpu_depth_state.should_write_mask = false;
        }

        gpu.color.push_state(gpu_color_state);
        gpu.depth.push_state(gpu_depth_state);

        //TODO: stencil mask
    }

	void FrameBuffer::set_size(const GpuFrameBufferSizeType& size) {
        if (size == get_size()) return;

        if (color_texture != nullptr) color_texture->set_size(size);
        if (depth_texture != nullptr) depth_texture->set_size(size);
        if (depth_stencil_texture != nullptr) depth_stencil_texture->set_size(size);

        this->size = size;
    }
}
