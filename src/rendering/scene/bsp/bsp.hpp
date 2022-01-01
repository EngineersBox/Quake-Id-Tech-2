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
#include "../../../device/gpu/gpu.hpp"
#include "../../../device/gpu/buffers/vertexBuffer.hpp"
#include "../../../device/gpu/buffers/indexBuffer.hpp"
#include "bspShader.hpp"
#include "../../view/cameraParams.hpp"

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
            int planeIndex = 0;
            std::array<short, CHILD_COUNT> childIndices;
            ::Scene::Structure::AABB3<short> aabb;
            unsigned short faceStartIndex = 0;
            unsigned short faceCount = 0;
        };

        struct Face {
            typedef unsigned char LightingStyleType;
            static const auto LIGHTING_STYLE_COUNT = 4;
            static const LightingStyleType LIGHTING_STYLE_NONE = 255;
            unsigned short planeIndex = 0;
            unsigned short planeSide = 0;
            unsigned int surfaceEdgeStartIndex = 0;
            unsigned short surfaceEdgeCount = 0;
            unsigned short textureInfoIndex = 0;
            std::array<LightingStyleType, LIGHTING_STYLE_COUNT> lightingStyles;
            unsigned int lightmapOffset = 0;
        };

        struct Leaf {
            static const auto AMBIENT_SOUND_LEVEL_COUNT = 4;
            typedef unsigned char AmbientSoundLevelType;
            typedef ::Scene::Structure::AABB3<short> AABBType;
            ContentType contentType = ContentType::EMPTY;
            int visibilityOffset = 0;
            AABBType aabb;
            unsigned short markSurfaceStartIndex = 0;
            unsigned short markSurfaceCount = 0;
            std::array<AmbientSoundLevelType, AMBIENT_SOUND_LEVEL_COUNT> ambientSoundLevels;
        };

        struct Edge {
            typedef unsigned short VertexIndexType;
            static const auto VERTEX_INDEX_COUNT = 2;
            std::array<VertexIndexType, VERTEX_INDEX_COUNT> vertexIndices;
        };

        struct TextureInfo {
            struct {
                glm::vec3 axis;
                float offset = 0;
            } s, t;
            unsigned int textureIndex = 0;
            unsigned int flags = 0;
        };

        struct ClipNode {
            static const auto CHILD_COUNT = 2;
            typedef short ChildIndexType;
            int planeIndex = 0;
            std::array<ChildIndexType, CHILD_COUNT> childIndices;
        };

        struct Model {
            static const auto HEAD_NODE_INDEX_COUNT = 4;
            ::Scene::Structure::AABB3<float> aabb;
            glm::vec3 origin;
            std::array<NodeIndexType, HEAD_NODE_INDEX_COUNT> headNodeIndices;
            int visLeafs = 0;
            int faceStartIndex = 0;
            int faceCount = 0;
        };

        struct BSPTexture {
            static const auto MIPMAP_OFFSET_COUNT = 4;
            unsigned int width;
            unsigned int height;
            unsigned int mipmapOffsets[MIPMAP_OFFSET_COUNT];
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

        typedef BSPShader::VertexType VertexType;
        typedef Device::GPU::Buffers::VertexBuffer<VertexType> VertexBufferType;
        typedef unsigned int IndexType;
        typedef Device::GPU::Buffers::IndexBuffer<IndexType> IndexBufferType;

        struct TraceArgs {
            ::Scene::Structure::Line3<float> line;
        };

        struct TraceResult {
            bool didHit = false;
            bool isAllSolid = false;
            glm::vec3 location;
            ::Scene::Structure::Plane3<float> plane;
            float ratio = 0.0f;
        };

        struct RenderSettings {
            float lightmap_gamma = 1.0f;
        };

        struct RenderStats {
            unsigned int faceCount = 0;
            unsigned int leafCount = 0;
            unsigned int leafIndex = 0;
            void reset() {
                this->faceCount = 0;
                this->leafCount = 0;
                this->leafIndex = 0;
            }
        };

        BSP(std::istream& istream);
        void render(const View::CameraParameters& cameraParameters);
        [[nodiscard]] int getLeafIndexFromLocation(const glm::vec3& location) const;
        [[nodiscard]] const RenderStats& geRenderStats() const { return this->renderStats; }
        RenderSettings renderSettings;  //TODO: sort this out elsewhere

    private:
        std::vector<BSPPlane> planes;
        std::vector<Edge> edges;
        std::vector<Face> faces;
        std::vector<int> surfaceEdges;
        std::vector<Node> nodes;
        std::vector<Leaf> leaves;
        std::vector<unsigned short> markSurfaces;
        std::vector<TextureInfo> textureInfos;
        std::vector<boost::shared_ptr<Resources::Texture>> faceLightmapTextures;
        std::vector<ClipNode> clipNodes;
        std::vector<Model> models;
        std::vector<BSPEntity> entities;
        std::vector<size_t> brushEntityIndices;
        std::map<size_t, boost::dynamic_bitset<>> leafPvsMap;
        std::vector<size_t> faceStartIndices;
        size_t visLeafCount = 0;
        std::vector<boost::shared_ptr<Resources::Texture>> textures;
        RenderStats renderStats;
        boost::shared_ptr<VertexBufferType> vertexBuffer;
        boost::shared_ptr<IndexBufferType> indexBuffer;

        BSP(const BSP&) = delete;
        BSP& operator=(const BSP&) = delete;
    };
}

#endif //QUAKE_BSP_HPP
