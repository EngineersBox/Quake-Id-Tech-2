#pragma once

//naga
#include "guiNode.hpp"
#include "../device/gpu/buffers/frameBuffer.hpp"
#include "../device/gpu/buffers/vertexBuffer.hpp"
#include "../device/gpu/buffers/indexBuffer.hpp"
#include "../device/gpu/shaders/programs/blurHorizontalShader.hpp"

namespace GUI {
	struct GUICanvas : GUINode {
		GUICanvas();

        static const auto INDEX_COUNT = 4;

        using VertexType = Device::GPU::Shaders::Programs::BlurHorizontalShader::VertexType;
        using VertexBufferType = Device::GPU::Buffers::VertexBuffer<VertexType>;
        using IndexType = Device::GPU::IndexType<INDEX_COUNT>::Type;
        using IndexBufferType = Device::GPU::Buffers::IndexBuffer<IndexType>;

        virtual void onRenderBegin(glm::mat4& world_matrix, glm::mat4& view_projection_matrix) override;
        virtual void onRenderEnd(glm::mat4& world_matrix, glm::mat4& view_projection_matrix) override;
        virtual void onCleanEnd() override;

		const boost::shared_ptr<Device::GPU::Buffers::FrameBuffer>& getFrameBuffer() const { return this->frameBuffer; }

    private:
        boost::shared_ptr<Device::GPU::Buffers::FrameBuffer> frameBuffer;
		static boost::weak_ptr<VertexBufferType> vertexBuffer;
		static boost::weak_ptr<IndexBufferType> indexBuffer;
    };
}