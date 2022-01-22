#include "gpu.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/make_shared.hpp>
#include <boost/crc.hpp>
#include <boost/filesystem.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>

#include "opengl.hpp"
#include "shaders/shader.hpp"
#include "buffers/frameBuffer.hpp"
#include "buffers/gpuBuffer.hpp"
#include "../../store/cache.hpp"
#include "../../resources/io/io.hpp"


namespace Device::GPU {
    Gpu gpu;

    inline GLenum getBufferTarget(Gpu::BufferTarget buffer_target) {
        switch (buffer_target) {
            case Gpu::BufferTarget::ARRAY:
                return GL_ARRAY_BUFFER;
            case Gpu::BufferTarget::ATOMIC_COUNTER:
                return GL_ATOMIC_COUNTER_BUFFER;
            case Gpu::BufferTarget::COPY_READ:
                return GL_COPY_READ_BUFFER;
            case Gpu::BufferTarget::COPY_WRITE:
                return GL_COPY_WRITE_BUFFER;
            case Gpu::BufferTarget::DISPATCH_INDIRECT:
                return GL_DISPATCH_INDIRECT_BUFFER;
            case Gpu::BufferTarget::DRAW_INDRECT:
                return GL_DRAW_INDIRECT_BUFFER;
            case Gpu::BufferTarget::ELEMENT_ARRAY:
                return GL_ELEMENT_ARRAY_BUFFER;
            case Gpu::BufferTarget::PIXEL_PACK:
                return GL_PIXEL_PACK_BUFFER;
            case Gpu::BufferTarget::PIXEL_UNPACK:
                return GL_PIXEL_UNPACK_BUFFER;
            case Gpu::BufferTarget::QUERY:
                return GL_QUERY_BUFFER;
            case Gpu::BufferTarget::SHADER_STORAGE:
                return GL_SHADER_STORAGE_BUFFER;
            default:
                throw std::invalid_argument("");
        }
    }

    inline GLenum getBufferUsage(Gpu::BufferUsage buffer_usage) {
        switch (buffer_usage) {
            case Gpu::BufferUsage::STREAM_DRAW:
                return GL_STREAM_DRAW;
            case Gpu::BufferUsage::STREAM_READ:
                return GL_STREAM_READ;
            case Gpu::BufferUsage::STREAM_COPY:
                return GL_STREAM_COPY;
            case Gpu::BufferUsage::STATIC_DRAW:
                return GL_STATIC_DRAW;
            case Gpu::BufferUsage::STATIC_READ:
                return GL_STATIC_READ;
            case Gpu::BufferUsage::STATIC_COPY:
                return GL_STATIC_COPY;
            case Gpu::BufferUsage::DYNAMIC_DRAW:
                return GL_DYNAMIC_DRAW;
            case Gpu::BufferUsage::DYNAMIC_READ:
                return GL_DYNAMIC_READ;
            case Gpu::BufferUsage::DYNAMIC_COPY:
                return GL_DYNAMIC_COPY;
            default:
                throw std::invalid_argument("");
        }
    }

    inline GLenum getPrimitiveType(Gpu::PrimitiveType primitive_type) {
        switch (primitive_type) {
            case Gpu::PrimitiveType::POINTS:
                return GL_POINTS;
            case Gpu::PrimitiveType::LINES:
                return GL_LINES;
            case Gpu::PrimitiveType::LINE_LOOP:
                return GL_LINE_LOOP;
            case Gpu::PrimitiveType::LINE_STRIP:
                return GL_LINE_STRIP;
            case Gpu::PrimitiveType::TRIANGLES:
                return GL_TRIANGLES;
            case Gpu::PrimitiveType::TRIANGLE_STRIP:
                return GL_TRIANGLE_STRIP;
            case Gpu::PrimitiveType::TRIANGLE_FAN:
                return GL_TRIANGLE_FAN;
            case Gpu::PrimitiveType::QUADS:
                return GL_QUADS;
            case Gpu::PrimitiveType::QUAD_STRIP:
                return GL_QUAD_STRIP;
            case Gpu::PrimitiveType::POLYGON:
                return GL_POLYGON;
            default:
                throw std::invalid_argument("");
        }
    }

    inline GLenum get_blend_factor(Gpu::BlendFactor blend_factor) {
        switch (blend_factor) {
            case Gpu::BlendFactor::ZERO:
                return GL_ZERO;
            case Gpu::BlendFactor::ONE:
                return GL_ONE;
            case Gpu::BlendFactor::SRC_COLOR:
                return GL_SRC_COLOR;
            case Gpu::BlendFactor::ONE_MINUS_SRC_COLOR:
                return GL_ONE_MINUS_SRC_COLOR;
            case Gpu::BlendFactor::DST_COLOR:
                return GL_DST_COLOR;
            case Gpu::BlendFactor::ONE_MINUS_DST_COLOR:
                return GL_ONE_MINUS_DST_COLOR;
            case Gpu::BlendFactor::SRC_ALPHA:
                return GL_SRC_ALPHA;
            case Gpu::BlendFactor::ONE_MINUS_SRC_ALPHA:
                return GL_ONE_MINUS_SRC_ALPHA;
            case Gpu::BlendFactor::DST_ALPHA:
                return GL_DST_ALPHA;
            case Gpu::BlendFactor::ONE_MINUS_DST_ALPHA:
                return GL_ONE_MINUS_DST_ALPHA;
            case Gpu::BlendFactor::CONSTANT_COLOR:
                return GL_CONSTANT_COLOR;
            case Gpu::BlendFactor::ONE_MINUS_CONSTANT_COLOR:
                return GL_ONE_MINUS_CONSTANT_COLOR;
            case Gpu::BlendFactor::CONSTANT_ALPHA:
                return GL_CONSTANT_ALPHA;
            case Gpu::BlendFactor::ONE_MINUS_CONSTANT_ALPHA:
                return GL_ONE_MINUS_CONSTANT_ALPHA;
            case Gpu::BlendFactor::SRC_ALPHA_SATURATE:
                return GL_SRC_ALPHA_SATURATE;
            default:
                throw std::invalid_argument("");
        }
    }

    inline GLenum get_blend_equation(Gpu::BlendEquation blend_equation) {
        switch (blend_equation) {
            case Gpu::BlendEquation::ADD:
                return GL_FUNC_ADD;
            case Gpu::BlendEquation::SUBTRACT:
                return GL_FUNC_SUBTRACT;
            case Gpu::BlendEquation::SUBTRACT_REVERSE:
                return GL_FUNC_REVERSE_SUBTRACT;
            case Gpu::BlendEquation::MIN:
                return GL_MIN;
            case Gpu::BlendEquation::MAX:
                return GL_MAX;
            default:
                throw std::invalid_argument("");
        }
    }

    inline GLenum get_depth_function(Gpu::DepthFunction depth_function) {
        switch (depth_function) {
            case Gpu::DepthFunction::NEVER:
                return GL_NEVER;
            case Gpu::DepthFunction::LESS:
                return GL_LESS;
            case Gpu::DepthFunction::EQUAL:
                return GL_EQUAL;
            case Gpu::DepthFunction::LEQUAL:
                return GL_LEQUAL;
            case Gpu::DepthFunction::GREATER:
                return GL_GREATER;
            case Gpu::DepthFunction::NOTEQUAL:
                return GL_NOTEQUAL;
            case Gpu::DepthFunction::GEQUAL:
                return GL_GEQUAL;
            case Gpu::DepthFunction::ALWAYS:
                return GL_ALWAYS;
            default:
                return GL_LESS;
        }
    }

    inline GLbitfield get_clear_flag_mask(GpuClearFlagType clear_flag) {
        switch (clear_flag) {
            case Gpu::CLEAR_FLAG_COLOR:
                return GL_COLOR_BUFFER_BIT;
            case Gpu::CLEAR_FLAG_DEPTH:
                return GL_DEPTH_BUFFER_BIT;
            case Gpu::CLEAR_FLAG_ACCUM:
                return GL_ACCUM_BUFFER_BIT;
            case Gpu::CLEAR_FLAG_STENCIL:
                return GL_STENCIL_BUFFER_BIT;
            default:
                throw std::invalid_argument("");
        }
    }

    inline GLenum get_stencil_function(Gpu::StencilFunction stencil_function) {
        switch (stencil_function) {
            case Gpu::StencilFunction::ALWAYS:
                return GL_ALWAYS;
            case Gpu::StencilFunction::EQUAL:
                return GL_EQUAL;
            case Gpu::StencilFunction::GEQUAL:
                return GL_GEQUAL;
            case Gpu::StencilFunction::GREATER:
                return GL_GREATER;
            case Gpu::StencilFunction::LEQUAL:
                return GL_LEQUAL;
            case Gpu::StencilFunction::LESS:
                return GL_LESS;
            case Gpu::StencilFunction::NEVER:
                return GL_NEVER;
            case Gpu::StencilFunction::NOTEQUAL:
                return GL_NOTEQUAL;
            default:
                throw std::invalid_argument("");
        }
    }

    inline GLenum get_stencil_operation(Gpu::StencilOperation stencil_operation) {
        switch (stencil_operation) {
            case Gpu::StencilOperation::DECR:
                return GL_DECR;
            case Gpu::StencilOperation::DECR_WRAP:
                return GL_DECR_WRAP;
            case Gpu::StencilOperation::INCR:
                return GL_INCR;
            case Gpu::StencilOperation::INCR_WRAP:
                return GL_INCR_WRAP;
            case Gpu::StencilOperation::INVERT:
                return GL_INVERT;
            case Gpu::StencilOperation::KEEP:
                return GL_KEEP;
            case Gpu::StencilOperation::REPLACE:
                return GL_REPLACE;
            case Gpu::StencilOperation::ZERO:
                return GL_ZERO;
            default:
                throw std::invalid_argument("");
        }
    }

    inline GLenum getDataType(GpuDataTypes data_type) {
        switch (data_type) {
            case GpuDataTypes::BYTE:
                return GL_BYTE;
            case GpuDataTypes::UNSIGNED_BYTE:
                return GL_UNSIGNED_BYTE;
            case GpuDataTypes::SHORT:
                return GL_SHORT;
            case GpuDataTypes::UNSIGNED_SHORT:
                return GL_UNSIGNED_SHORT;
            case GpuDataTypes::INT:
                return GL_INT;
            case GpuDataTypes::UNSIGNED_INT:
                return GL_UNSIGNED_INT;
            case GpuDataTypes::FLOAT:
                return GL_FLOAT;
            case GpuDataTypes::DOUBLE:
                return GL_DOUBLE;
            default:
                throw std::invalid_argument("");
        }
    }

    inline GLenum getShaderType(Gpu::ShaderType shader_type) {
        switch (shader_type) {
            case Gpu::ShaderType::FRAGMENT:
                return GL_FRAGMENT_SHADER;
            case Gpu::ShaderType::VERTEX:
                return GL_VERTEX_SHADER;
            default:
                throw std::invalid_argument("");
        }
    }

    void getTextureFormats(ColorType color_type, Resources::Texture::FormatType& internal_format, Resources::Texture::FormatType& format, Resources::Texture::TypeType& type) {
        switch (color_type) {
            case ColorType::G:
                format = GL_LUMINANCE;
                internal_format = 1;
                type = GL_UNSIGNED_BYTE;
                break;
            case ColorType::RGB:
                format = GL_RGB;
                internal_format = GL_RGB;
                type = GL_UNSIGNED_BYTE;
                break;
            case ColorType::RGBA:
                format = GL_RGBA;
                internal_format = GL_RGBA;
                type = GL_UNSIGNED_BYTE;
                break;
            case ColorType::GA:
                format = GL_LUMINANCE_ALPHA;
                internal_format = 2;
                type = GL_UNSIGNED_BYTE;
                break;
            case ColorType::DEPTH:
                format = GL_DEPTH_COMPONENT;
                internal_format = GL_DEPTH_COMPONENT;
                type = GL_UNSIGNED_BYTE;
                break;
            case ColorType::DEPTH_STENCIL:
                format = GL_DEPTH_STENCIL;
                internal_format = GL_DEPTH24_STENCIL8;
                type = GL_UNSIGNED_INT_24_8;
                break;
            default:
                throw std::exception();
        }
    }

    size_t getBytesPerPixel(ColorType color_type) {
        switch (color_type) {
            case ColorType::G:
                return 1;
            case ColorType::RGB:
                return 3;
            case ColorType::RGBA:
                return 4;
            case ColorType::GA:
                return 2;
            case ColorType::DEPTH:
                return 1;
            case ColorType::DEPTH_STENCIL:
                return 4;
            default:
                throw std::exception();
        }
    }

    void Gpu::clear(const GpuClearFlagType clearFlags) const {
        GLbitfield clearMask = 0;

        auto buildClearMask = [&](const GpuClearFlagType clearFlag) -> void {
            if ((clearFlags & clearFlag) == clearFlag) {
                clearMask |= get_clear_flag_mask(clearFlag);
            }
        };

        buildClearMask(CLEAR_FLAG_COLOR);
        buildClearMask(CLEAR_FLAG_DEPTH);
        buildClearMask(CLEAR_FLAG_ACCUM);
        buildClearMask(CLEAR_FLAG_STENCIL);

        glClear(clearMask); glCheckError();
    }

    GpuId Gpu::createBuffer() {
        GpuId id;
        glGenBuffers(1, &id); glCheckError();
        return id;
    }

    void Gpu::destroyBuffer(GpuId id) {
        glDeleteBuffers(1, &id); glCheckError();
    }

    GpuId Gpu::createFrameBuffer(GpuFrameBufferType type, const GpuFrameBufferSizeType& size, boost::shared_ptr<Resources::Texture>& colorTexture, boost::shared_ptr<Resources::Texture>& depthStencilTexture, boost::shared_ptr<Resources::Texture>& depthTexture) {
        GpuId id;

        glGenFramebuffers(1, &id); glCheckError();
        glBindFramebuffer(GL_FRAMEBUFFER, id); glCheckError();

        GpuFrameBufferTypeFlagsType typeFlags = static_cast<GpuFrameBufferTypeFlagsType>(type);

        //color
        if ((typeFlags & GPU_FRAME_BUFFER_TYPE_FLAG_COLOR) == GPU_FRAME_BUFFER_TYPE_FLAG_COLOR) {
            colorTexture = boost::make_shared<Resources::Texture>(ColorType::RGB, size, nullptr);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture->get_id(), 0); glCheckError();
        } else {
            glDrawBuffer(GL_NONE); glCheckError();
            glReadBuffer(GL_NONE); glCheckError();
        }

        //Depth & stencil
        if ((typeFlags & (GPU_FRAME_BUFFER_TYPE_FLAG_DEPTH | GPU_FRAME_BUFFER_TYPE_FLAG_STENCIL)) == (GPU_FRAME_BUFFER_TYPE_FLAG_DEPTH | GPU_FRAME_BUFFER_TYPE_FLAG_STENCIL)) {
            depthStencilTexture = boost::make_shared<Resources::Texture>(ColorType::DEPTH_STENCIL, size, nullptr);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencilTexture->get_id(), 0); glCheckError();
        } else if ((typeFlags & GPU_FRAME_BUFFER_TYPE_FLAG_DEPTH) == GPU_FRAME_BUFFER_TYPE_FLAG_DEPTH) {
            //Depth
            depthTexture = boost::make_shared<Resources::Texture>(ColorType::DEPTH, size, nullptr);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture->get_id(), 0); glCheckError();
        }

        //restore previously bound frame buffer
        boost::optional<boost::weak_ptr<Buffers::FrameBuffer>> frameBuffer = gpu.frameBufferManager.top();
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer ? (int) frameBuffer->lock()->getId() : 0); glCheckError();

        return id;
    }

    void Gpu::destroyFrameBuffer(GpuId id) {
        glDeleteFramebuffers(1, &id);
    }

    GpuId Gpu::createTexture(ColorType color_type, glm::uvec2 size, const void* data) {
        size = glm::max(glm::uvec2(1), size);

        GpuId id;
        glGenTextures(1, &id); glCheckError();
        glBindTexture(GL_TEXTURE_2D, id); glCheckError();

        GLint unpackAlignment;
        glGetIntegerv(GL_UNPACK_ALIGNMENT, &unpackAlignment); glCheckError();
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); glCheckError();

        Resources::Texture::FormatType internalFormat, format;
        Resources::Texture::TypeType type;
        getTextureFormats(color_type, internalFormat, format, type);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glCheckError();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); glCheckError();
        glTexImage2D(
                GL_TEXTURE_2D,
                0,
                internalFormat,
                size.x,
                size.y,
                0,
                format,
                type,
                data
        ); glCheckError();
        glPixelStorei(GL_UNPACK_ALIGNMENT, unpackAlignment); glCheckError();
        glBindTexture(GL_TEXTURE_2D, 0); glCheckError();

        return id;
    }

    void Gpu::resizeTexture(const boost::shared_ptr<Resources::Texture>& texture, glm::uvec2 size) {
        Resources::Texture::FormatType internalFormat, format;
        Resources::Texture::TypeType type;
        size = glm::max(glm::uvec2(1), size);
        getTextureFormats(texture->getColorType(), internalFormat, format, type);
        glBindTexture(GL_TEXTURE_2D, texture->get_id()); glCheckError();
        glTexImage2D(
                GL_TEXTURE_2D,
                0,
                internalFormat,
                size.x,
                size.y,
                0,
                format,
                type,
                nullptr
        ); glCheckError();
        glBindTexture(GL_TEXTURE_2D, 0); glCheckError();
    }

    void Gpu::destroyTexture(GpuId id) {
        glDeleteTextures(1, &id); glCheckError();
    }

    GpuLocation Gpu::getUniformLocation(GpuId program_id, const char* name) const {
        int uniformLocation = glGetUniformLocation(program_id, name); glCheckError();
        return uniformLocation;
    }

    GpuLocation Gpu::getAttributeLocation(GpuId program_id, const char* name) const {
        int attributeLocation = glGetAttribLocation(program_id, name); glCheckError();
        return attributeLocation;
    }

    void Gpu::getUniform(const char* name, std::vector<glm::mat4>& params, size_t count) {
        params.resize(count);
        const ValueWrapper<unsigned int, 0> programId = this->programs.top()->lock()->getId();
        glGetnUniformfvARB(programId, getUniformLocation(programId, name), static_cast<GLsizei>(count * sizeof(glm::mat4)), reinterpret_cast<GLfloat*>(&params[0]));
    }

    void Gpu::enableVertexAttributeArray(GpuLocation location) {
        glEnableVertexAttribArray(location); glCheckError();
    }

    void Gpu::disableVertexAttributeArray(GpuLocation location) {
        glDisableVertexAttribArray(location); glCheckError();
    }

    void Gpu::setVertexAttribPointer(GpuLocation location, int size, GpuDataTypes dataType, bool isNormalized, int stride, const void* pointer) {
        glVertexAttribPointer(location, size, getDataType(dataType), isNormalized, stride, pointer); glCheckError();
    }

    void Gpu::setVertexAttribPointer(GpuLocation location, int size, GpuDataTypes dataType, int stride, const void * pointer) {
        glVertexAttribIPointer(location, size, getDataType(dataType), stride, pointer); glCheckError();
    }

    void Gpu::setUniform(const char* name, const glm::mat3& value, bool shouldTranpose) const {
        glUniformMatrix3fv(getUniformLocation(this->programs.top()->lock()->getId(), name), 1, shouldTranpose, glm::value_ptr(value)); glCheckError();
    }

    void Gpu::setUniform(const char* name, const glm::mat4& value, bool shouldTranpose) const {
        glUniformMatrix4fv(getUniformLocation(this->programs.top()->lock()->getId(), name), 1, shouldTranpose, glm::value_ptr(value)); glCheckError();
    }

    void Gpu::setUniform(const char* name, int value) const {
        glUniform1i(getUniformLocation(this->programs.top()->lock()->getId(), name), value); glCheckError();
    }

    void Gpu::setUniform(const char* name, unsigned int value) const {
        glUniform1i(getUniformLocation(this->programs.top()->lock()->getId(), name), value); glCheckError();
    }

    void Gpu::setUniform(const char* name, float value) const {
        glUniform1f(getUniformLocation(this->programs.top()->lock()->getId(), name), value); glCheckError();
    }

    void Gpu::setUniform(const char* name, const glm::vec2& value) const {
        glUniform2fv(getUniformLocation(this->programs.top()->lock()->getId(), name), 1, glm::value_ptr(value)); glCheckError();
    }

    void Gpu::setUniform(const char* name, const glm::vec3& value) const {
        glUniform3fv(getUniformLocation(this->programs.top()->lock()->getId(), name), 1, glm::value_ptr(value)); glCheckError();
    }

    void Gpu::setUniform(const char* name, const glm::vec4& value) const {
        glUniform4fv(getUniformLocation(this->programs.top()->lock()->getId(), name), 1, glm::value_ptr(value)); glCheckError();
    }

    void Gpu::setUniform(const char* name, const std::vector<glm::mat4>& value, bool shouldTranspose) const {
        const GLfloat* f = reinterpret_cast<const GLfloat*>(value.data());
        float d = f[17];
        ValueWrapper<int, -1> loc = getUniformLocation(this->programs.top()->lock()->getId(), name);
        glUniformMatrix4fv(loc, static_cast<GLsizei>(value.size()), shouldTranspose ? GL_TRUE : GL_FALSE, f); glCheckError();
    }

    void Gpu::setUniformSubroutine(ShaderType shaderType, GpuIndex index) {
        glUniformSubroutinesuiv(getShaderType(shaderType), 1, &index); glCheckError();
    }

    void Gpu::setClearColor(glm::vec4 & _color) {
        glClearColor(_color.r, _color.g, _color.b, _color.a); glCheckError();
    }

    glm::vec4 Gpu::getClearColor() {
        glm::vec4 clearColor;
        glGetFloatv(GL_COLOR_CLEAR_VALUE, glm::value_ptr(clearColor)); glCheckError();
        return clearColor;
    }

    GpuLocation Gpu::getSubroutineUniformLocation(GpuId programId, ShaderType shaderType, const std::string& name) {
        int location = glGetSubroutineUniformLocation(programId, getShaderType(shaderType), name.c_str()); glCheckError();
        return location;
    }

    GpuIndex Gpu::getSubroutineIndex(GpuId programId, ShaderType shaderType, const std::string& name) {
        unsigned int index = glGetSubroutineIndex(programId, getShaderType(shaderType), name.c_str()); glCheckError();
        return index;
    }

    boost::optional<Gpu::ProgramManager::WeakType> Gpu::ProgramManager::top() const {
        boost::optional<WeakType> gpuShader;
        if (!programs.empty()) {
            gpuShader = programs.top();
        }
        return gpuShader;
    }

    void Gpu::ProgramManager::push(const Gpu::ProgramManager::WeakType& program) {
        if (programs.empty() || program.lock() != programs.top().lock()) {
            glUseProgram(program.lock()->getId()); glCheckError();
        }
        programs.push(program);
        program.lock()->onBind();
    }

    Gpu::ProgramManager::WeakType Gpu::ProgramManager::pop() {
        if (programs.empty()) {
            throw std::exception();
        }

        boost::weak_ptr<Shaders::Shader> previousProgram = programs.top();
        programs.pop();
        previousProgram.lock()->onUnbind();

        if (programs.empty()) {
            glUseProgram(0); glCheckError();
            return {};
        } else {
            glUseProgram(programs.top().lock()->getId()); glCheckError();
            return programs.top();
        }
    }

    boost::optional<Gpu::FrameBufferManager::WeakType> Gpu::FrameBufferManager::top() const {
        boost::optional<WeakType> frameBuffer;
        if (!frameBuffers.empty()) {
            frameBuffer = frameBuffers.top();
        }
        return frameBuffer;
    }

    void Gpu::FrameBufferManager::push(const Gpu::FrameBufferManager::SharedType& frame_buffer) {
        if (frameBuffers.empty() || frame_buffer != frameBuffers.top()) {
            glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer->getId()); glCheckError();
            frame_buffer->on_bind();
        }
        frameBuffers.push(frame_buffer);
    }

    Gpu::FrameBufferManager::WeakType Gpu::FrameBufferManager::pop() {
        if (frameBuffers.empty()) {
            throw std::out_of_range("");
        }

        frameBuffers.pop();
        if (frameBuffers.empty()) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0); glCheckError();
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); glCheckError();
            glDepthMask(GL_TRUE); glCheckError();
            //TODO: stencil mask
            return {};
        } else {
            glBindFramebuffer(GL_FRAMEBUFFER, frameBuffers.top()->getId()); glCheckError();
            frameBuffers.top()->on_bind();
            return frameBuffers.top();
        }
    }

    Gpu::TextureManager::WeakType Gpu::TextureManager::get(IndexType index) const {
        if (index < 0 || index >= textureCount) {
            throw std::out_of_range("");
        }
        return textures[index];
    }

    Gpu::TextureManager::WeakType Gpu::TextureManager::bind(IndexType index, const SharedType& texture) {
        const boost::shared_ptr<Resources::Texture>& previousTexture = textures[index];
        if (previousTexture == texture) return previousTexture;

        glActiveTexture(GL_TEXTURE0 + index); glCheckError();
        glBindTexture(GL_TEXTURE_2D, texture != nullptr ? (int) texture->get_id() : 0); glCheckError();
        textures[index] = texture;
        return previousTexture;
    }

    Gpu::TextureManager::WeakType Gpu::TextureManager::unbind(IndexType index) {
        const boost::shared_ptr<Resources::Texture>& previousTexture = textures[index];
        if (previousTexture == nullptr) return previousTexture;
        glActiveTexture(GL_TEXTURE0 + index); glCheckError();
        glBindTexture(GL_TEXTURE_2D, 0); glCheckError();
        textures[index] = nullptr;
        return previousTexture;
    }

    GpuViewportType Gpu::ViewportManager::top() const {
        if (viewports.empty()) {
            glm::ivec4 viewport;
            glGetIntegerv(GL_VIEWPORT, glm::value_ptr(viewport)); glCheckError();
            return {viewport.x, viewport.y, viewport.z, viewport.w};
        }
        return viewports.top();
    }

    void Gpu::ViewportManager::push(const GpuViewportType& viewport) {
        viewports.push(viewport);
        glViewport(
                static_cast<GLint>(viewport.x),
                static_cast<GLint>(viewport.y),
                static_cast<GLint>(viewport.width),
                static_cast<GLint>(viewport.height)
        ); glCheckError();
    }

    GpuViewportType Gpu::ViewportManager::pop() {
        if (viewports.empty()) throw std::exception();
        const Scenes::Structure::Rectangle<float>& previousViewport = viewports.top();

        viewports.pop();
        const Scenes::Structure::Rectangle<int> topViewport = static_cast<Scenes::Structure::Rectangle<int>>(top());
        glViewport(
                topViewport.x,
                topViewport.y,
                topViewport.width,
                topViewport.height
        ); glCheckError();
        return previousViewport;
    }

    void Gpu::BufferManager::put(BufferType & buffer) {
        buffers.insert(buffer);
    }

    void Gpu::BufferManager::erase(BufferType & buffer) {
        buffers.erase(buffer);
    }
    
    void Gpu::BufferManager::push(BufferTarget target, BufferType buffer) {
        if (targetBuffers.find(target) == targetBuffers.end()) {
            targetBuffers.emplace(std::make_pair(target, std::stack<BufferType>()));
        }
        targetBuffers[target].push(buffer);
        glBindBuffer(getBufferTarget(target), buffer->getId()); glCheckError();
    }

    Gpu::BufferManager::BufferType Gpu::BufferManager::pop(BufferTarget target) {
        auto targetBuffersItr = targetBuffers.find(target);
        if (targetBuffersItr == targetBuffers.end()) {
            throw std::out_of_range("");
        }

        std::stack<boost::shared_ptr<Buffers::GpuBuffer>>& _buffers = targetBuffersItr->second;
        if (_buffers.empty()) {
            throw std::out_of_range("");
        }

        _buffers.pop();
        glBindBuffer(getBufferTarget(target), _buffers.empty() ? 0 : (int) _buffers.top()->getId()); glCheckError();
        return _buffers.empty() ? BufferType() : _buffers.top();
    }


    Gpu::BufferManager::BufferType Gpu::BufferManager::top(BufferTarget target) const {
        auto targetBuffersItr = targetBuffers.find(target);

        if (targetBuffersItr == targetBuffers.end()) {
            throw std::out_of_range("");
        }

        const std::stack<boost::shared_ptr<Buffers::GpuBuffer>>& _buffers = targetBuffersItr->second;
        if (_buffers.empty()) {
            throw std::out_of_range("");
        }
        return _buffers.top();
    }

    void Gpu::BufferManager::data(BufferTarget target, const void* data, size_t size, BufferUsage usage) {
        glBufferData(getBufferTarget(target), size, data, getBufferUsage(usage)); glCheckError();
    }
    
    //TODO: infer indexDataType from bound index buffer
    void Gpu::drawElements(PrimitiveType primitiveType, size_t count, GpuDataTypes indexDataType, size_t offset) const {
        glDrawElements(
                getPrimitiveType(primitiveType),
                static_cast<GLsizei>(count),
                getDataType(indexDataType),
                reinterpret_cast<GLvoid*>(offset)
        ); glCheckError();
    }

    GpuId Gpu::createProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource) const {
        auto createShader = [&](GLenum type, const std::string& source) -> GLint {
            //create shaders
            GpuId id = glCreateShader(type); glCheckError();

            const char* strings = source.c_str();
            GLint lengths[1] = { static_cast<GLint>(source.length()) };

            glShaderSource(id, 1, &strings, &lengths[0]); glCheckError();
            glCompileShader(id); glCheckError();

            GLsizei compileStatus;
            glGetShaderiv(id, GL_COMPILE_STATUS, &compileStatus); glCheckError();

            if (compileStatus == GL_FALSE) {
#if defined(DEBUG)
                GLsizei shaderInfoLogLength = 0;
                GLchar shaderInfoLog[GL_INFO_LOG_LENGTH] = { '\0' };

                glGetShaderInfoLog(id, GL_INFO_LOG_LENGTH, &shaderInfoLogLength, &shaderInfoLog[0]); glCheckError();
                spdlog::debug(shaderInfoLog);
#endif
                glDeleteShader(id); glCheckError();
                throw std::exception();
            }

            return id;
        };

        //create program
        const unsigned int id = glCreateProgram(); glCheckError();
        boost::crc_32_type crc32;
        crc32.process_bytes(vertexShaderSource.data(), vertexShaderSource.size());
        crc32.process_bytes(fragmentShaderSource.data(), fragmentShaderSource.size());
        unsigned int sourceChecksum = crc32.checksum();

        //TODO: give these programs proper names
        std::unique_ptr<std::basic_ifstream<char>> ifstream = Store::cache.get(std::to_string(sourceChecksum));

        if (ifstream->is_open()) {
            GLenum binaryFormat;
            GLsizei binaryLength;
            std::vector<char> binary;

            Resources::IO::read(*ifstream, binaryFormat);
            Resources::IO::read(*ifstream, binaryLength);
            Resources::IO::read(*ifstream, binary, binaryLength);

            glProgramBinary(id, binaryFormat, binary.data(), binaryLength); glCheckError();

            //link status
            GLint linkStatus;
            glGetProgramiv(id, GL_LINK_STATUS, &linkStatus); glCheckError();
            if (linkStatus == GL_TRUE) return id;

            GLint programInfoLogLength = 0;
            glGetProgramiv(id, GL_INFO_LOG_LENGTH, &programInfoLogLength); glCheckError();

            if (programInfoLogLength > 0) {
                std::string programInfoLog;
                programInfoLog.resize(programInfoLogLength);
                glGetProgramInfoLog(id, programInfoLogLength, nullptr, &programInfoLog[0]); glCheckError();
                spdlog::debug(programInfoLog);
            }
        }

        const int vertexShader = createShader(GL_VERTEX_SHADER, vertexShaderSource);
        const int fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

        glAttachShader(id, vertexShader); glCheckError();
        glAttachShader(id, fragmentShader); glCheckError();
        glLinkProgram(id); glCheckError();

        GLint linkStatus;
        glGetProgramiv(id, GL_LINK_STATUS, &linkStatus); glCheckError();

        if (linkStatus == GL_FALSE) {
            GLint programInfoLogLength = 0;
            glGetProgramiv(id, GL_INFO_LOG_LENGTH, &programInfoLogLength); glCheckError();

            if (programInfoLogLength > 0) {
                std::string programInfoLog;
                programInfoLog.resize(programInfoLogLength);
                glGetProgramInfoLog(id, programInfoLogLength, nullptr, &programInfoLog[0]); glCheckError();
                spdlog::debug(programInfoLog);
            }
            glDeleteProgram(id); glCheckError();
            throw std::exception();
        }

        glDetachShader(id, vertexShader); glCheckError();
        glDetachShader(id, fragmentShader); glCheckError();

        GLsizei binaryLength = 0;
        glGetProgramiv(id, GL_PROGRAM_BINARY_LENGTH, &binaryLength); glCheckError();

        GLenum binaryFormat = 0;
        std::vector<unsigned char> programBinaryData(binaryLength);
        glGetProgramBinary(id, static_cast<GLsizei>(programBinaryData.size()), &binaryLength, &binaryFormat, static_cast<GLvoid*>(programBinaryData.data())); glCheckError();

        std::stringstream stringstream;
        Resources::IO::write(stringstream, binaryFormat);
        Resources::IO::write(stringstream, binaryLength);
        Resources::IO::write(stringstream, programBinaryData);
        std::string data = stringstream.str();

        Store::cache.put_buffer(std::to_string(sourceChecksum), data.data(), data.size());
        return id;
    }

    void Gpu::destroyProgram(GpuId id) {
        glDeleteProgram(id); glCheckError();
    }

    //blend
    Gpu::BlendStateManager::BlendState Gpu::BlendStateManager::getState() const {
        if (!states.empty()) return states.top();
        return {};
    }

	void Gpu::BlendStateManager::pushState(const Gpu::BlendStateManager::BlendState& state) {
        applyState(state);
        states.push(state);
    }

    void Gpu::BlendStateManager::popState() {
        states.pop();
        applyState(getState());
    }

    void Gpu::BlendStateManager::applyState(const Gpu::BlendStateManager::BlendState& state) {
        if (state.isEnabled) {
            glEnable(GL_BLEND); glCheckError();
        } else {
            glDisable(GL_BLEND); glCheckError();
        }
        glBlendFunc(get_blend_factor(state.srcFactor), get_blend_factor(state.dstFactor)); glCheckError();
        glBlendEquation(get_blend_equation(state.equation)); glCheckError();
    }

    //Depth
    Gpu::Depth::State Gpu::Depth::getState() const {
        if (!states.empty()) {
            return states.top();
        }
        return {};
    }

    void Gpu::Depth::pushState(const State& state) {
        apply_state(state);
        states.push(state);
    }

    void Gpu::Depth::popState() {
        states.pop();
        apply_state(getState());
    }

    void Gpu::Depth::apply_state(const State& state) {
        if (state.shouldTest) {
            glEnable(GL_DEPTH_TEST); glCheckError();
        } else {
            glDisable(GL_DEPTH_TEST); glCheckError();
        }
        glDepthMask(state.shouldWriteMask ? GL_TRUE : GL_FALSE); glCheckError();
        glDepthFunc(get_depth_function(state.function)); glCheckError();
    }

    //culling
    Gpu::CullingStateManager::CullingState Gpu::CullingStateManager::getState() const {
        if (!states.empty()) {
            return states.top();
        }
        return {};
    }

    void Gpu::CullingStateManager::pushState(const CullingState& state) {
        applyState(state);
        states.push(state);
    }

    void Gpu::CullingStateManager::popState() {
        states.pop();
        applyState(getState());
    }

    void Gpu::CullingStateManager::applyState(const CullingState& state) {
        if (state.isEnabled) {
            glEnable(GL_CULL_FACE); glCheckError();
        } else {
            glDisable(GL_CULL_FACE); glCheckError();
        }

        switch (state.frontFace) {
            case CullingFrontFace::CCW:
                glFrontFace(GL_CCW); glCheckError();
                break;
            case CullingFrontFace::CW:
                glFrontFace(GL_CW); glCheckError();
                break;
        }
        
        switch (state.mode) {
            case CullingMode::BACK:
                glCullFace(GL_BACK); glCheckError();
                break;
            case CullingMode::FRONT:
                glCullFace(GL_FRONT); glCheckError();
                break;
            case CullingMode::FRONT_AND_BACK:
                glCullFace(GL_FRONT_AND_BACK); glCheckError();
                break;
        }
    }

    //stencil
    Gpu::StencilStateManager::StencilState Gpu::StencilStateManager::getState() const {
        if (!states.empty()) {
            return states.top();
        }
        return {};
    }

    void Gpu::StencilStateManager::pushState(const StencilState& state) {
        applyState(state);
        states.push(state);
    }

    void Gpu::StencilStateManager::popState() {
        states.pop();
        applyState(getState());
    }

    void Gpu::StencilStateManager::applyState(const StencilState& state) {
        if (state.isEnabled) {
            glEnable(GL_STENCIL_TEST); glCheckError();
        } else {
            glDisable(GL_STENCIL_TEST); glCheckError();
        }

        glStencilFunc(get_stencil_function(state.function.func), state.function.ref, state.function.mask); glCheckError();
        glStencilOp(get_stencil_operation(state.operations.fail), get_stencil_operation(state.operations.zfail), get_stencil_operation(state.operations.zpass)); glCheckError();
        glStencilMask(state.mask); glCheckError();
    }

    //color
    Gpu::ColorStateManager::ColorState Gpu::ColorStateManager::getState() const {
        if (!states.empty()) {
            return states.top();
        }
        return {};
    }

    void Gpu::ColorStateManager::pushState(const ColorState& state) {
        applyState(state);
        states.push(state);
    }

    void Gpu::ColorStateManager::popState() {
        states.pop();
        applyState(getState());
    }

    void Gpu::ColorStateManager::applyState(const ColorState& state) {
        glColorMask(
                state.mask.r ? GL_TRUE : GL_FALSE,
                state.mask.g ? GL_TRUE : GL_FALSE,
                state.mask.b ? GL_TRUE : GL_FALSE,
                state.mask.a ? GL_TRUE : GL_FALSE
        ); glCheckError();
    }

    const std::string& Gpu::getVendor() const {
        static const std::string VENDOR = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
        return VENDOR;
    }

    const std::string& Gpu::getRenderer() const {
        static const std::string RENDERER = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
        return RENDERER;
    }

    const std::string& Gpu::getVersion() const {
        static const std::string VERSION = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        return VERSION;
    }

    const std::string& Gpu::getShadingLanguageVersion() const {
        static const std::string SHADING_LANGUAGE_VERISON = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
        return SHADING_LANGUAGE_VERISON;
    }

    const std::string& Gpu::getExtensions() const {
        static const std::string EXTENSIONS = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
        return EXTENSIONS;
    }

    void Gpu::getTextureData(const boost::shared_ptr<Resources::Texture>& texture, std::vector<unsigned char>& data, int level) {
        int internalFormat, format, type;
        getTextureFormats(texture->getColorType(), internalFormat, format, type);
        const unsigned long bytes_per_pixel = getBytesPerPixel(texture->getColorType());
        data.resize(texture->getWidth() * texture->getHeight() * bytes_per_pixel);
        this->textures.bind(0, texture);
        glGetTexImage(GL_TEXTURE_2D, level, format, type, data.data());
        this->textures.unbind(0);
    }

    std::unique_ptr<unsigned char[]> Gpu::getBackbufferPixels(int& width, int& height) {
        Scenes::Structure::Rectangle<float> viewport = this->viewports.top();
        width = static_cast<int>(viewport.width);
        height = static_cast<int>(viewport.height);
        std::unique_ptr<unsigned char[]> pixels(new unsigned char[width * height * 4]);
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, static_cast<GLvoid*>(pixels.get()));
        return pixels;
    }
}