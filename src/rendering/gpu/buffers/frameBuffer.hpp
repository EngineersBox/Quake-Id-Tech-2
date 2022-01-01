#pragma once

#include <map>
#include <boost/shared_ptr.hpp>

#include "../gpuDefs.hpp"
#include "../../../resources/texture.hpp"

namespace Rendering::GPU::Buffers {
    struct FrameBuffer {
		[[nodiscard]] GpuId getId() const { return id; }
        [[nodiscard]] const boost::shared_ptr<Resources::Texture>& getColorTexture() const { return colorTexture; }
		[[nodiscard]] const boost::shared_ptr<Resources::Texture>& getDepthTexture() const { return depthTexture; }
		[[nodiscard]] const boost::shared_ptr<Resources::Texture>& getDepthStencilTexture() const { return depthStencilTexture; }
        [[nodiscard]] const GpuFrameBufferSizeType& getSize() const { return size; }
        [[nodiscard]] GpuFrameBufferType getType() const { return type; }

        void setSize(const GpuFrameBufferSizeType& _size);

		FrameBuffer(GpuFrameBufferType type, const GpuFrameBufferSizeType& size);
		~FrameBuffer();

        void on_bind() const;

    private:
        GpuId id;
		boost::shared_ptr<Resources::Texture> colorTexture;
		boost::shared_ptr<Resources::Texture> depthTexture;
		boost::shared_ptr<Resources::Texture> depthStencilTexture;
        GpuFrameBufferSizeType size;
        GpuFrameBufferType type;

		FrameBuffer(const FrameBuffer&) = delete;
		FrameBuffer& operator=(const FrameBuffer&) = delete;
    };
}
