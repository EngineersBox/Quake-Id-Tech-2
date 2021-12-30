#pragma once

#ifndef QUAKE_BSP_HPP
#define QUAKE_BSP_HPP

#include <array>
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>

#include "../../../scene/structure/aabb.hpp"
#include "../../../scene/structure/line.hpp"
#include "../../../resources/texture.hpp"
#include "bspEntity.hpp"
#include "../../gpu/gpu.hpp"
#include "../../../rendering/gpu/vertex_buffer.hpp"
#include "../../../rendering/gpu/index_buffer.hpp"

namespace Rendering::Scene {
class BSP: public Resources::Resource {
    public:
        typedef int NodeIndexType;
        enum class ContentType: int {
            EMPTY = -1,
            SOLID = -2,
            WATER = -3,
            SLIME = -4,
            LAVA = -5,
            SKY = -6,
            ORIGIN = -7,
            CLIP = -8,
            CURRENT_0 = -9,
            CURRENT_90 = -10,
            CURRENT_180 = -11,
            CURRENT_270 = -12,
            CURRENT_UP = -13,
            CURRENT_DOWN = -14,
            TRANSLUCENT = -15
        };

        enum class RenderMode: unsigned char {
            NORMAL,
            COLOR,
            TEXTURE,
            GLOW,
            SOLID,
            ADDITIVE
        };

        struct Node {
            static const auto CHILD_COUNT = 2;
            int plane_index = 0;
            std::array<short, CHILD_COUNT> child_indices;
            ::Scene::Structure::AABB3<short> aabb;
            unsigned short face_start_index = 0;
            unsigned short face_count = 0;
        };

        struct Face {
            typedef unsigned char LightingStyleType;
            static const auto LIGHTING_STYLE_COUNT = 4;
            static const LightingStyleType LIGHTING_STYLE_NONE = 255;
            unsigned short plane_index = 0;
            unsigned short plane_side = 0;
            unsigned int surface_edge_start_index = 0;
            unsigned short surface_edge_count = 0;
            unsigned short texture_info_index = 0;
            std::array<LightingStyleType, LIGHTING_STYLE_COUNT> lighting_styles;
            unsigned int lightmap_offset = 0;
        };

        struct Leaf {
            static const auto AMBIENT_SOUND_LEVEL_COUNT = 4;
            typedef unsigned char AmbientSoundLevelType;
            typedef ::Scene::Structure::AABB3<short> AABBType;
            ContentType content_type = ContentType::EMPTY;
            int visibility_offset = 0;
            AABBType aabb;
            unsigned short mark_surface_start_index = 0;
            unsigned short mark_surface_count = 0;
            std::array<AmbientSoundLevelType, AMBIENT_SOUND_LEVEL_COUNT> ambient_sound_levels;
        };

        struct Edge {
            typedef unsigned short VertexIndexType;
            static const auto VERTEX_INDEX_COUNT = 2;
            std::array<VertexIndexType, VERTEX_INDEX_COUNT> vertex_indices;
        };

        struct TextureInfo {
            struct {
                glm::vec3 axis;
                float offset = 0;
            } s, t;
            unsigned int texture_index = 0;
            unsigned int flags = 0;
        };

        struct ClipNode {
            static const auto CHILD_COUNT = 2;
            typedef short ChildIndexType;
            int plane_index = 0;
            std::array<ChildIndexType, CHILD_COUNT> child_indices;
        };

        struct Model {
            static const auto HEAD_NODE_INDEX_COUNT = 4;
            ::Scene::Structure::AABB3<float> aabb;
            glm::vec3 origin;
            std::array<NodeIndexType, HEAD_NODE_INDEX_COUNT> head_node_indices;
            int vis_leafs = 0;
            int face_start_index = 0;
            int face_count = 0;
        };

        struct BSPTexture {
            static const auto MIPMAP_OFFSET_COUNT = 4;
            unsigned int width;
            unsigned int height;
            unsigned int mipmap_offsets[MIPMAP_OFFSET_COUNT];
        };

        struct BSPPlane {
            typedef ::Scene::Structure::Plane3<float> PlaneType;
            enum class Type: unsigned int {
                X,
                Y,
                Z,
                ANY_X,
                ANY_Y,
                ANY_Z
            };
            PlaneType plane;
            Type type = Type::X;
        };

        typedef bsp_gpu_program::VertexType VertexType;
        typedef Rendering::GPU::VertexBuffer<VertexType> VertexBufferType;
        typedef unsigned int IndexType;
        typedef Rendering::GPU::IndexBuffer<IndexType> IndexBufferType;

        struct TraceArgs {
            ::Scene::Structure::Line3<float> line;
        };

        struct TraceResult {
            bool did_hit = false;
            bool is_all_solid = false;
            glm::vec3 location;
            ::Scene::Structure::Plane3<float> plane;
            float ratio = 0.0f;
        };

        struct RenderSettings {
            float lightmap_gamma = 1.0f;
        };

        struct RenderStats {
            unsigned int face_count = 0;
            unsigned int leaf_count = 0;
            unsigned int leaf_index = 0;
            void reset() {
                face_count = 0;
                leaf_count = 0;
                leaf_index = 0;
            }
        };

        BSP(std::istream& istream);
        void render(const CameraParameters& camera_parameters);
        [[nodiscard]] int get_leaf_index_from_location(const glm::vec3& location) const;
        [[nodiscard]] const RenderStats& geRenderStats() const { return render_stats; }
        RenderSettings render_settings;  //TODO: sort this out elsewhere

    private:
        std::vector<BSPPlane> planes;
        std::vector<Edge> edges;
        std::vector<Face> faces;
        std::vector<int> surface_edges;
        std::vector<Node> nodes;
        std::vector<Leaf> leafs;
        std::vector<unsigned short> mark_surfaces;
        std::vector<TextureInfo> texture_infos;
        std::vector<boost::shared_ptr<Resources::Texture>> face_lightmap_textures;
        std::vector<ClipNode> clip_nodes;
        std::vector<Model> models;
        std::vector<BSPEntity> entities;
        std::vector<size_t> brush_entity_indices;
        std::map<size_t, boost::dynamic_bitset<>> leaf_pvs_map;
        std::vector<size_t> face_start_indices;
        size_t vis_leaf_count = 0;
        std::vector<boost::shared_ptr<Resources::Texture>> textures;
        RenderStats render_stats;
        boost::shared_ptr<VertexBufferType> vertex_buffer;
        boost::shared_ptr<IndexBufferType> index_buffer;

        BSP(const BSP&) = delete;
        BSP& operator=(const BSP&) = delete;
    };
}

#endif //QUAKE_BSP_HPP
