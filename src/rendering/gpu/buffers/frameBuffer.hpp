#pragma once

#include <map>
#include <boost/shared_ptr.hpp>

#include "../gpuDefs.hpp"
#include "../../../resources/texture.hpp"

namespace Rendering::GPU::Buffers {
    struct FrameBuffer {
		[[nodiscard]] GpuId get_id() const { return id; }
        [[nodiscard]] const boost::shared_ptr<Resources::Texture>& get_color_texture() const { return color_texture; }
		[[nodiscard]] const boost::shared_ptr<Resources::Texture>& get_depth_texture() const { return depth_texture; }
		[[nodiscard]] const boost::shared_ptr<Resources::Texture>& get_depth_stencil_texture() const { return depth_stencil_texture; }
        [[nodiscard]] const GpuFrameBufferSizeType& get_size() const { return size; }
        [[nodiscard]] GpuFrameBufferType get_type() const { return type; }

        void set_size(const GpuFrameBufferSizeType& size);

		FrameBuffer(GpuFrameBufferType type, const GpuFrameBufferSizeType& size);
		~FrameBuffer();

        void on_bind() const;

    private:
        GpuId id;
		boost::shared_ptr<Resources::Texture> color_texture;
		boost::shared_ptr<Resources::Texture> depth_texture;
		boost::shared_ptr<Resources::Texture> depth_stencil_texture;
        GpuFrameBufferSizeType size;
        GpuFrameBufferType type;

		FrameBuffer(const FrameBuffer&) = delete;
		FrameBuffer& operator=(const FrameBuffer&) = delete;
    };
}
