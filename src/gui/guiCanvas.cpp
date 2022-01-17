#include <fstream>
#include <boost/make_shared.hpp>
#include <glm/ext.hpp>

#include "guiCanvas.hpp"
#include "../device/gpu/shaders/shaderManager.hpp"
#include "../device/gpu/buffers/gpuBufferManager.hpp"
#include "../core/application/app.hpp"

namespace GUI {
    boost::weak_ptr<GUICanvas::IndexBufferType> GUICanvas::indexBuffer;
    boost::weak_ptr<GUICanvas::VertexBufferType> GUICanvas::vertexBuffer;

    GUICanvas::GUICanvas() {
        if (GUICanvas::indexBuffer.expired()) {
            GUICanvas::indexBuffer = Device::GPU::Buffers::gpuBuffers.make<IndexBufferType>().lock();
            GUI::GUICanvas::indexBuffer.lock()->data({ 0, 1, 2, 3 }, Device::GPU::Gpu::BufferUsage::STATIC_DRAW);
        }

        if (GUICanvas::vertexBuffer.expired()) {
            auto vertices = {
                VertexType(glm::vec3(0, 0, 0), glm::vec2(0, 0)),
                VertexType(glm::vec3(1, 0, 0), glm::vec2(1, 0)),
                VertexType(glm::vec3(1, 1, 0), glm::vec2(1, 1)),
                VertexType(glm::vec3(0, 1, 0), glm::vec2(0, 1))
            };
            GUICanvas::vertexBuffer = Device::GPU::Buffers::gpuBuffers.make<VertexBufferType>().lock();
            GUICanvas::vertexBuffer.lock()->data(vertices, Device::GPU::Gpu::BufferUsage::STATIC_DRAW);
        }
    }

    void GUICanvas::onRenderBegin(glm::mat4& world_matrix, glm::mat4& view_projection_matrix) {
        Device::GPU::GpuViewportType viewport;
        viewport.width = static_cast<Device::GPU::GpuViewportType::ScalarType>(getSize().x);
        viewport.height = static_cast<Device::GPU::GpuViewportType::ScalarType>(getSize().y);
        viewport.x = static_cast<Device::GPU::GpuViewportType::ScalarType>(getBounds().min.x);
        viewport.y = static_cast<Device::GPU::GpuViewportType::ScalarType>(getBounds().min.y);
        Device::GPU::gpu.frameBufferManager.push(frameBuffer);
        Device::GPU::gpu.viewports.push(viewport);
    }

    void GUICanvas::onRenderEnd(glm::mat4& world_matrix, glm::mat4& view_projection_matrix) {
        static const unsigned int DIFFUSE_TEXTURE_INDEX = 0;

        Device::GPU::gpu.viewports.pop();
        Device::GPU::gpu.frameBufferManager.pop();

        //TODO: for each render pass, push/pop frame buffer, do gpu program etc.

        Device::GPU::gpu.buffers.push(Device::GPU::Gpu::BufferTarget::ARRAY, vertexBuffer.lock());
        Device::GPU::gpu.buffers.push(Device::GPU::Gpu::BufferTarget::ELEMENT_ARRAY, indexBuffer.lock());

        const auto shader = Device::GPU::Shaders::shaders.get<Device::GPU::Shaders::Programs::BlurHorizontalShader>();
        Device::GPU::gpu.programs.push(shader);

        auto gpuWorldMatrix = world_matrix;
        gpuWorldMatrix *= glm::translate(glm::mat4(), glm::vec3(getBounds().min.x, getBounds().min.y, 0.0f));
        gpuWorldMatrix *= glm::scale(glm::mat4(), glm::vec3(getSize().x, getSize().y, 1.0f));   //TODO: verify correctness

        Device::GPU::gpu.setUniform("world_matrix", gpuWorldMatrix);
        Device::GPU::gpu.setUniform("view_projection_matrix", view_projection_matrix);
        Device::GPU::gpu.setUniform("diffuse_texture", DIFFUSE_TEXTURE_INDEX);
        Device::GPU::gpu.setUniform("t", Core::Application::app.getUptimeSeconds());

        Device::GPU::gpu.textures.bind(DIFFUSE_TEXTURE_INDEX, frameBuffer->getColorTexture());
        Device::GPU::gpu.drawElements(Device::GPU::Gpu::PrimitiveType::TRIANGLE_FAN, 4, IndexBufferType::DATA_TYPE, 0);
        Device::GPU::gpu.textures.unbind(DIFFUSE_TEXTURE_INDEX);

        Device::GPU::gpu.buffers.pop(Device::GPU::Gpu::BufferTarget::ELEMENT_ARRAY);
        Device::GPU::gpu.buffers.pop(Device::GPU::Gpu::BufferTarget::ARRAY);
        Device::GPU::gpu.programs.pop();
    }

    void GUICanvas::onCleanEnd() {
        const auto frameBufferSize = static_cast<Device::GPU::GpuFrameBufferSizeType>(getSize());

        if (!frameBuffer) {
            frameBuffer = boost::make_shared<Device::GPU::Buffers::FrameBuffer>(Device::GPU::GpuFrameBufferType::COLOR_DEPTH_STENCIL, frameBufferSize);
        } else if(frameBufferSize != frameBuffer->getSize()) {
            frameBuffer->setSize(frameBufferSize);
        }
    }
}
