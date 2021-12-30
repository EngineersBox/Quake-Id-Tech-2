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
#include "gpu_defs.hpp"
#include "colorTypes.hpp"
#include "../../resources/texture.hpp"

namespace Rendering::GPU {
	struct FrameBuffer;
	struct GpuBuffer;
    struct GpuProgram;

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
            typedef boost::weak_ptr<GpuProgram> WeakType;
			[[nodiscard]] boost::optional<WeakType> top() const;
			void push(const WeakType& data);
			WeakType pop();

        private:
			std::stack<WeakType> programs;
        } programs;

        //frame buffers
        struct FrameBufferManager {
			typedef boost::weak_ptr<FrameBuffer> WeakType;
			typedef boost::shared_ptr<FrameBuffer> SharedType;
            [[nodiscard]] boost::optional<WeakType> top() const;
			void push(const SharedType& frame_buffer);
			WeakType pop();

        private:
			std::stack<SharedType> frame_buffers;
        } frame_buffers;

        //textures
        struct TextureManager {
            const static auto texture_count = 32;
            typedef boost::weak_ptr<Resources::Texture> WeakType;
			typedef boost::shared_ptr<Resources::Texture> SharedType;
            typedef IndexType<texture_count>::Type IndexType;
			[[nodiscard]] WeakType get(IndexType index) const;
			WeakType bind(IndexType index, const SharedType& texture);
			WeakType unbind(IndexType index);

        private:
            std::array<SharedType, texture_count> textures;
        } textures;

        //viewports
        struct viewport_mgr {
            [[nodiscard]] GpuViewportType top() const;
            void push(const GpuViewportType& viewport);
            GpuViewportType pop();
        private:
            std::stack<GpuViewportType> viewports;
        } viewports;

        //buffers
        struct BufferManager {
            typedef boost::shared_ptr<GpuBuffer> BufferType;
			void put(BufferType& buffer);
			void erase(BufferType& buffer);
			void push(BufferTarget target, BufferType buffer);
			BufferType pop(BufferTarget target);
			[[nodiscard]] BufferType top(BufferTarget target) const;
			void data(BufferTarget target, const void* data, size_t size, BufferUsage usage);
        private:
			std::map<BufferTarget, std::stack<BufferType>> target_buffers;
			std::set<BufferType> buffers;
        } buffers;

        //blend
        struct BlendStateManager {
            struct BlendState {
                bool is_enabled = false;
				BlendFactor src_factor = BlendFactor::ONE;
				BlendFactor dst_factor = BlendFactor::ZERO;
				BlendEquation equation = BlendEquation::ADD;
            };

			[[nodiscard]] BlendState get_state() const;
			void push_state(const BlendState& state);
            void pop_state();
        private:
			std::stack<BlendState> states;
			void apply_state(const BlendState& state);
        } blend;

        //depth
        struct depth {
            struct state {
                bool should_test = false;
                bool should_write_mask = true;
				DepthFunction function = DepthFunction::DEFAULT;
            };

            state get_state() const;
            void push_state(const state& state);
            void pop_state();
        private:
            std::stack<state> states;
            void apply_state(const state& state);
        } depth;

        struct CullingStateManager {
            struct CullingState {
                bool is_enabled = false;
				CullingFrontFace front_face = CullingFrontFace::CCW;
				CullingMode mode = CullingMode::BACK;
            };

			[[nodiscard]] CullingState get_state() const;
			void push_state(const CullingState& state);
            void pop_state();
        private:
			std::stack<CullingState> states;
			void apply_state(const CullingState& state);
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
                bool is_enabled = false;
                unsigned int mask = 0xFFFFFFFF;
            };
			[[nodiscard]] StencilState get_state() const;
			void push_state(const StencilState& state);
            void pop_state();
        private:
			std::stack<StencilState> states;
			void apply_state(const StencilState& state);
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
			[[nodiscard]] ColorState get_state() const;
			void push_state(const ColorState& state);
            void pop_state();
        private:
			std::stack<ColorState> states;
			void apply_state(const ColorState& state);
        } color;

        void clear(GpuClearFlagType clear_flags) const;
        void draw_elements(PrimitiveType primitive_type, size_t count, GpuDataTypes index_data_type, size_t offset) const;

        [[nodiscard]] GpuId create_program(const std::string& vertex_shader_source, const std::string& fragment_shader_source) const;
		void destroy_program(GpuId id);

		GpuId create_buffer();
		void destroy_buffer(GpuId id);

		GpuId create_frame_buffer(GpuFrameBufferType type, const GpuFrameBufferSizeType& size, boost::shared_ptr<Resources::Texture>& color_texture, boost::shared_ptr<Resources::Texture>& depth_stencil_texture, boost::shared_ptr<Resources::Texture>& depth_texture);
		void destroy_frame_buffer(GpuId id);

		GpuId create_texture(ColorType color_type, glm::uvec2 size, const void* data);
		void resize_texture(const boost::shared_ptr<Resources::Texture>& texture, glm::uvec2 size);
		void destroy_texture(GpuId id);

		GpuLocation get_uniform_location(GpuId program_id, const char* name) const;
		GpuLocation get_attribute_location(GpuId program_id, const char* name) const;

		void get_uniform(const char* name, std::vector<glm::mat4>& params, size_t count);

        void enable_vertex_attribute_array(GpuLocation location);
		void disable_vertex_attribute_array(GpuLocation location);
		void set_vertex_attrib_pointer(GpuLocation location, int size, GpuDataTypes data_type, bool is_normalized, int stride, const void* pointer);
		void set_vertex_attrib_pointer(GpuLocation location, int size, GpuDataTypes data_type, int stride, const void* pointer);
        void set_uniform(const char* name, const glm::mat3& value, bool should_tranpose = false) const;
        void set_uniform(const char* name, const glm::mat4& value, bool should_tranpose = false) const;
        void set_uniform(const char* name, int value) const;
        void set_uniform(const char* name, unsigned int value) const;
        void set_uniform(const char* name, float value) const;
        void set_uniform(const char* name, const glm::vec2& value) const;
        void set_uniform(const char* name, const glm::vec3& value) const;
        void set_uniform(const char* name, const glm::vec4& value) const;
        void set_uniform(const char* name, const std::vector<glm::mat4>& value, bool should_transpose = false) const;
        void set_uniform_subroutine(ShaderType shader_type, GpuIndex index);

        void set_clear_color(glm::vec4& color);
        glm::vec4 get_clear_color();

        GpuLocation get_subroutine_uniform_location(GpuId program_id, ShaderType shader_type, const std::string& name);
        GpuIndex get_subroutine_index(GpuId program_id, ShaderType shader_type, const std::string& name);

        [[nodiscard]] const std::string& get_vendor() const;
        [[nodiscard]] const std::string& get_renderer() const;
        [[nodiscard]] const std::string& get_version() const;
        [[nodiscard]] const std::string& get_shading_language_version() const;
        [[nodiscard]] const std::string& get_extensions() const;

		void get_texture_data(const boost::shared_ptr<Resources::Texture>& texture, std::vector<unsigned char>& data, int level = 0);

		std::unique_ptr<unsigned char[]> get_backbuffer_pixels(int& width, int& height);
    };

    extern Gpu gpu;
}