#pragma once

#include <stack>
#include <array>
#include <vector>
#include <map>
#include <set>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <glm/glm.hpp>

#include "../../scene/structure/rectangle.hpp"
#include "indexType.hpp"
#include "gpuDefs.hpp"
#include "colorTypes.hpp"
#include "../../resources/texture.hpp"
#include "buffers/frameBuffer.hpp"
//#include "shaders/shader.hpp"
#include "buffers/gpuBuffer.hpp"

namespace Device::GPU {
    namespace Shaders {
        struct Shader;
    }

    struct Gpu {
        enum class BufferTarget {
            ARRAY,
            ATOMIC_COUNTER,
            COPY_READ,
            COPY_WRITE,
            DRAW_INDRECT,
            DISPATCH_INDIRECT,
            ELEMENT_ARRAY,
            PIXEL_PACK,
            PIXEL_UNPACK,
            QUERY,
            SHADER_STORAGE
        };

        enum class BufferUsage {
            STREAM_DRAW,
            STREAM_READ,
            STREAM_COPY,
            STATIC_DRAW,
            STATIC_READ,
            STATIC_COPY,
            DYNAMIC_DRAW,
            DYNAMIC_READ,
            DYNAMIC_COPY
        };

        enum class PrimitiveType {
            POINTS,
            LINES,
            LINE_LOOP,
            LINE_STRIP,
            TRIANGLES,
            TRIANGLE_STRIP,
            TRIANGLE_FAN,
            QUADS,
            QUAD_STRIP,
            POLYGON
        };

        enum class BlendFactor {
            ZERO,
            ONE,
            SRC_COLOR,
            ONE_MINUS_SRC_COLOR,
            DST_COLOR,
            ONE_MINUS_DST_COLOR,
            SRC_ALPHA,
            ONE_MINUS_SRC_ALPHA,
            DST_ALPHA,
            ONE_MINUS_DST_ALPHA,
            CONSTANT_COLOR,
            ONE_MINUS_CONSTANT_COLOR,
            CONSTANT_ALPHA,
            ONE_MINUS_CONSTANT_ALPHA,
            SRC_ALPHA_SATURATE,
            DEFAULT = ONE
        };

        enum class BlendEquation {
            ADD,
            SUBTRACT,
            SUBTRACT_REVERSE,
            MIN,
            MAX,
            DEFAULT = ADD
        };

        enum class CullFace {
            FRONT,
            BACK,
            FRONT_AND_BACK,
            DEFAULT = BACK
        };

        enum class DepthFunction {
            NEVER,
            LESS,
            EQUAL,
            LEQUAL,
            GREATER,
            NOTEQUAL,
            GEQUAL,
            ALWAYS,
            DEFAULT = LESS
        };

        enum class CullingFrontFace {
            CW,
            CCW
        };

        enum class CullingMode {
            FRONT,
            BACK,
            FRONT_AND_BACK
        };

        enum ClearFlag : GpuClearFlagType {
            CLEAR_FLAG_COLOR = (1 << 0),
            CLEAR_FLAG_DEPTH = (1 << 1),
            CLEAR_FLAG_ACCUM = (1 << 2),
            CLEAR_FLAG_STENCIL = (1 << 3)
        };

        enum class StencilFunction {
            NEVER,
            LESS,
            LEQUAL,
            GREATER,
            GEQUAL,
            EQUAL,
            NOTEQUAL,
            ALWAYS
        };

        enum class StencilOperation {
            KEEP,
            ZERO,
            REPLACE,
            INCR,
            INCR_WRAP,
            DECR,
            DECR_WRAP,
            INVERT
        };

        enum class ShaderType {
            FRAGMENT,
            VERTEX
        };

        //programs
        struct ProgramManager {
            typedef boost::weak_ptr<Shaders::Shader> WeakType;
            [[nodiscard]] boost::optional<WeakType> top() const;
            void push(const WeakType& data);
            WeakType pop();

        private:
            std::stack<WeakType> programs;
        } programs;

        //frame buffers
        struct FrameBufferManager {
			typedef boost::weak_ptr<Buffers::FrameBuffer> WeakType;
			typedef boost::shared_ptr<Buffers::FrameBuffer> SharedType;
            [[nodiscard]] boost::optional<WeakType> top() const;
			void push(const SharedType& frame_buffer);
			WeakType pop();

        private:
            std::stack<SharedType> frameBuffers;
        } frameBufferManager;

        //textures
        struct TextureManager {
            const static auto textureCount = 32;
            typedef boost::weak_ptr<Resources::Texture> WeakType;
			typedef boost::shared_ptr<Resources::Texture> SharedType;
            typedef IndexType<textureCount>::Type IndexType;
			[[nodiscard]] WeakType get(IndexType index) const;
			WeakType bind(IndexType index, const SharedType& texture);
			WeakType unbind(IndexType index);

        private:
            std::array<SharedType, textureCount> textures;
        } textures;

        //viewports
        struct ViewportManager {
            [[nodiscard]] GpuViewportType top() const;
            void push(const GpuViewportType& viewport);
            GpuViewportType pop();
        private:
            std::stack<GpuViewportType> viewports;
        } viewports;

        //buffers
        struct BufferManager {
            typedef boost::shared_ptr<Buffers::GpuBuffer> BufferType;
			void put(BufferType& buffer);
			void erase(BufferType& buffer);
			void push(BufferTarget target, BufferType buffer);
			BufferType pop(BufferTarget target);
			[[nodiscard]] BufferType top(BufferTarget target) const;
			void data(BufferTarget target, const void* data, size_t size, BufferUsage usage);
        private:
			std::map<BufferTarget, std::stack<BufferType>> targetBuffers;
			std::set<BufferType> buffers;
        } buffers;

        //blend
        struct BlendStateManager {
            struct BlendState {
                bool isEnabled = false;
				BlendFactor srcFactor = BlendFactor::ONE;
				BlendFactor dstFactor = BlendFactor::ZERO;
				BlendEquation equation = BlendEquation::ADD;
            };

			[[nodiscard]] BlendState getState() const;
			void pushState(const BlendState& state);
            void popState();
        private:
			std::stack<BlendState> states;
			void applyState(const BlendState& state);
        } blend;

        //Depth
        struct Depth {
            struct State {
                bool shouldTest = false;
                bool shouldWriteMask = true;
				DepthFunction function = DepthFunction::DEFAULT;
            };

            [[nodiscard]] State getState() const;
            void pushState(const State& state);
            void popState();
        private:
            std::stack<State> states;
            void apply_state(const State& state);
        } depth;

        struct CullingStateManager {
            struct CullingState {
                bool isEnabled = false;
				CullingFrontFace frontFace = CullingFrontFace::CCW;
				CullingMode mode = CullingMode::BACK;
            };

			[[nodiscard]] CullingState getState() const;
			void pushState(const CullingState& state);
            void popState();
        private:
			std::stack<CullingState> states;
			void applyState(const CullingState& state);
        } culling;

        //stencil
        struct StencilStateManager {
            struct StencilState {
                struct StencilFunctionParameters {
					StencilFunction func = StencilFunction::ALWAYS;
                    int ref = 0;
                    unsigned int mask = 0xFFFFFFFF;
                };

                struct StencilOperations {
					StencilOperation fail = StencilOperation::KEEP;
					StencilOperation zfail = StencilOperation::KEEP;
					StencilOperation zpass = StencilOperation::KEEP;
                };

				StencilFunctionParameters function;
				StencilOperations operations;
                bool isEnabled = false;
                unsigned int mask = 0xFFFFFFFF;
            };
			[[nodiscard]] StencilState getState() const;
			void pushState(const StencilState& state);
            void popState();
        private:
			std::stack<StencilState> states;
			void applyState(const StencilState& state);
        } stencil;

        //color
        struct ColorStateManager {
            struct ColorState {
                struct ColorMask {
                    bool r = true;
                    bool g = true;
                    bool b = true;
                    bool a = true;
                };
				ColorMask mask;
            };
			[[nodiscard]] ColorState getState() const;
			void pushState(const ColorState& state);
            void popState();
        private:
			std::stack<ColorState> states;
			void applyState(const ColorState& state);
        } color;

        void clear(GpuClearFlagType clearFlag) const;
        void drawElements(PrimitiveType primitiveType, size_t count, GpuDataTypes indexDataType, size_t offset) const;

        [[nodiscard]] GpuId createProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource) const;
		void destroyProgram(GpuId id);

		GpuId createBuffer();
		void destroyBuffer(GpuId id);

		GpuId createFrameBuffer(GpuFrameBufferType type, const GpuFrameBufferSizeType& size, boost::shared_ptr<Resources::Texture>& colorTexture, boost::shared_ptr<Resources::Texture>& depthStencilTexture, boost::shared_ptr<Resources::Texture>& depthTexture);
		void destroyFrameBuffer(GpuId id);

		GpuId createTexture(ColorType color_type, glm::uvec2 size, const void* data);
		void resizeTexture(const boost::shared_ptr<Resources::Texture>& texture, glm::uvec2 size);
		void destroyTexture(GpuId id);

		GpuLocation getUniformLocation(GpuId program_id, const char* name) const;
		GpuLocation getAttributeLocation(GpuId program_id, const char* name) const;

		void getUniform(const char* name, std::vector<glm::mat4>& params, size_t count);

        void enableVertexAttributeArray(GpuLocation location);
		void disableVertexAttributeArray(GpuLocation location);
		void setVertexAttribPointer(GpuLocation location, int size, GpuDataTypes dataType, bool isNormalized, int stride, const void* pointer);
		void setVertexAttribPointer(GpuLocation location, int size, GpuDataTypes dataType, int stride, const void* pointer);
        void setUniform(const char* name, const glm::mat3& value, bool shouldTranpose = false) const;
        void setUniform(const char* name, const glm::mat4& value, bool shouldTranpose = false) const;
        void setUniform(const char* name, int value) const;
        void setUniform(const char* name, unsigned int value) const;
        void setUniform(const char* name, float value) const;
        void setUniform(const char* name, const glm::vec2& value) const;
        void setUniform(const char* name, const glm::vec3& value) const;
        void setUniform(const char* name, const glm::vec4& value) const;
        void setUniform(const char* name, const std::vector<glm::mat4>& value, bool shouldTranspose = false) const;
        void setUniformSubroutine(ShaderType shaderType, GpuIndex index);

        void setClearColor(glm::vec4& _color);
        glm::vec4 getClearColor();

        GpuLocation getSubroutineUniformLocation(GpuId programId, ShaderType shaderType, const std::string& name);
        GpuIndex getSubroutineIndex(GpuId programId, ShaderType shaderType, const std::string& name);

        [[nodiscard]] const std::string& getVendor() const;
        [[nodiscard]] const std::string& getRenderer() const;
        [[nodiscard]] const std::string& getVersion() const;
        [[nodiscard]] const std::string& getShadingLanguageVersion() const;
        [[nodiscard]] const std::string& getExtensions() const;

		void getTextureData(const boost::shared_ptr<Resources::Texture>& texture, std::vector<unsigned char>& data, int level = 0);

		std::unique_ptr<unsigned char[]> getBackbufferPixels(int& width, int& height);
    };

    extern Gpu gpu;
}
