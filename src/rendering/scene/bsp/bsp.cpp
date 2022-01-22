#include <iostream>
#include <glm/ext.hpp>
#include <boost/algorithm/string.hpp>
#include <exception>
#include <spdlog/spdlog.h>

#include "bsp.hpp"
#include "../../../platform/game/components/cameraParams.hpp"
#include "../../../device/gpu/shaders/programs/bspShader.hpp"
#include "../../../resources/resourceManager.hpp"
#include "../../../device/gpu/shaders/shaderManager.hpp"
#include "../../../device/gpu/buffers/gpuBufferManager.hpp"
#include "../../../resources/io/io.hpp"


namespace Rendering::Scene {
    struct BSPChunk {
        enum class Type: unsigned char {
            ENTITIES,
            PLANES,
            TEXTURES,
            VERTICES,
            VISIBLIITY,
            NODES,
            TEXTURE_INFO,
            FACES,
            LIGHTING,
            CLIP_NODES,
            LEAVES,
            MARK_SURFACES,
            EDGES,
            SURFACE_EDGES,
            MODELS,
            COUNT
        };

        unsigned int offset = 0;
        unsigned int length = 0;
    };

    BSP::BSP(std::istream& istream) {
        if (!istream.good()) throw std::runtime_error("Stream handle was not good");

        //version
        int version;
        Resources::IO::read(istream, version);
        static const auto BSP_VERSION = 30;
        if (version != BSP_VERSION) throw std::runtime_error("Bad BSP version: " + std::to_string(version));

        //chunks
        std::vector<BSPChunk> chunks;
        chunks.resize(static_cast<size_t>(BSPChunk::Type::COUNT));
        for (BSPChunk& chunk : chunks) {
            Resources::IO::read(istream, chunk.offset);
            Resources::IO::read(istream, chunk.length);
        }

        //planes
        const BSPChunk& planeChunk = chunks[static_cast<size_t>(BSPChunk::Type::PLANES)];
        istream.seekg(planeChunk.offset, std::ios_base::beg);

        unsigned long planeCount = planeChunk.length / sizeof(BSPPlane);
        this->planes.resize(planeCount);

        for (BSPPlane& plane : this->planes) {
            Resources::IO::read(istream, plane.plane.normal.x);
            Resources::IO::read(istream, plane.plane.normal.z);
            Resources::IO::read(istream, plane.plane.normal.y);
            Resources::IO::read(istream, plane.plane.distance);
            Resources::IO::read(istream, plane.type);
            plane.plane.normal.z = -plane.plane.normal.z;
        }

        //vertexLocations
        const BSPChunk& verticesChunk = chunks[static_cast<size_t>(BSPChunk::Type::VERTICES)];
        istream.seekg(verticesChunk.offset, std::ios_base::beg);

        std::vector<glm::vec3> vertexLocations;
        unsigned long vertexLocationCount = verticesChunk.length / sizeof(glm::vec3);
        vertexLocations.resize(vertexLocationCount);

        for (glm::vec<3, float>& vertexLocation : vertexLocations) {
            Resources::IO::read(istream, vertexLocation.x);
            Resources::IO::read(istream, vertexLocation.z);
            Resources::IO::read(istream, vertexLocation.y);
            vertexLocation.z = -vertexLocation.z;
        }

        //edges
        const BSPChunk& edgesChunk = chunks[static_cast<size_t>(BSPChunk::Type::EDGES)];
        istream.seekg(edgesChunk.offset, std::ios_base::beg);
        unsigned long edgeCount = edgesChunk.length / sizeof(Edge);
        this->edges.resize(edgeCount);

        for (Edge& edge : this->edges) {
            Resources::IO::read(istream, edge.vertexIndices[0]);
            Resources::IO::read(istream, edge.vertexIndices[1]);
        }

        //surfaceEdges
        const BSPChunk& surfaceEdgesChunk = chunks[static_cast<size_t>(BSPChunk::Type::SURFACE_EDGES)];
        istream.seekg(surfaceEdgesChunk.offset, std::ios_base::beg);
        unsigned long surfaceEdgeCount = surfaceEdgesChunk.length / sizeof(int);
        this->surfaceEdges.resize(surfaceEdgeCount);

        for (int& surfaceEdge : this->surfaceEdges) {
            Resources::IO::read(istream, surfaceEdge);
        }

        //faces
        const BSPChunk& facesChunk = chunks[static_cast<size_t>(BSPChunk::Type::FACES)];
        istream.seekg(facesChunk.offset, std::ios_base::beg);
        unsigned long faceCount = facesChunk.length / sizeof(Face);
        this->faces.resize(faceCount);

        for (Face& face : this->faces) {
            Resources::IO::read(istream, face.planeIndex);
            Resources::IO::read(istream, face.planeSide);
            Resources::IO::read(istream, face.surfaceEdgeStartIndex);
            Resources::IO::read(istream, face.surfaceEdgeCount);
            Resources::IO::read(istream, face.textureInfoIndex);
            Resources::IO::read(istream, face.lightingStyles[0]);
            Resources::IO::read(istream, face.lightingStyles[1]);
            Resources::IO::read(istream, face.lightingStyles[2]);
            Resources::IO::read(istream, face.lightingStyles[3]);
            Resources::IO::read(istream, face.lightmapOffset);
        }

        //nodes
        const BSPChunk& nodesChunk = chunks[static_cast<size_t>(BSPChunk::Type::NODES)];
        istream.seekg(nodesChunk.offset, std::ios_base::beg);
        unsigned long nodeCount = nodesChunk.length / sizeof(Node);
        this->nodes.resize(nodeCount);

        for (Node& node : this->nodes) {
            Resources::IO::read(istream, node.planeIndex);
            Resources::IO::read(istream, node.childIndices[0]);
            Resources::IO::read(istream, node.childIndices[1]);
            Resources::IO::read(istream, node.aabb.min.x);
            Resources::IO::read(istream, node.aabb.min.z);
            Resources::IO::read(istream, node.aabb.min.y);
            Resources::IO::read(istream, node.aabb.max.x);
            Resources::IO::read(istream, node.aabb.max.z);
            Resources::IO::read(istream, node.aabb.max.y);
            Resources::IO::read(istream, node.faceStartIndex);
            Resources::IO::read(istream, node.faceCount);
            node.aabb.min.z = -node.aabb.min.z;
            node.aabb.max.z = -node.aabb.max.z;
        }

        //leaves
        const BSPChunk& leavesChunk = chunks[static_cast<size_t>(BSPChunk::Type::LEAVES)];
        istream.seekg(leavesChunk.offset, std::ios_base::beg);
        unsigned long leafCount = leavesChunk.length / sizeof(Leaf);
        this->leaves.resize(leafCount);

        for (Leaf& leaf : this->leaves) {
            Resources::IO::read(istream, leaf.contentType);
            Resources::IO::read(istream, leaf.visibilityOffset);
            Resources::IO::read(istream, leaf.aabb.min.x);
            Resources::IO::read(istream, leaf.aabb.min.z);
            Resources::IO::read(istream, leaf.aabb.min.y);
            Resources::IO::read(istream, leaf.aabb.max.x);
            Resources::IO::read(istream, leaf.aabb.max.z);
            Resources::IO::read(istream, leaf.aabb.max.y);
            Resources::IO::read(istream, leaf.markSurfaceStartIndex);
            Resources::IO::read(istream, leaf.markSurfaceCount);
            Resources::IO::read(istream, leaf.ambientSoundLevels[0]);
            Resources::IO::read(istream, leaf.ambientSoundLevels[1]);
            Resources::IO::read(istream, leaf.ambientSoundLevels[2]);
            Resources::IO::read(istream, leaf.ambientSoundLevels[3]);

            leaf.aabb.min.z = -leaf.aabb.min.z;
            leaf.aabb.max.z = -leaf.aabb.max.z;
        }

        //markSurfaces
        const BSPChunk& markSurfacesChunk = chunks[static_cast<size_t>(BSPChunk::Type::MARK_SURFACES)];
        istream.seekg(markSurfacesChunk.offset, std::ios_base::beg);
        unsigned long markSurfaceCount = markSurfacesChunk.length / sizeof(unsigned short);
        this->markSurfaces.resize(markSurfaceCount);

        for (unsigned short& markSurface : this->markSurfaces) {
            Resources::IO::read(istream, markSurface);
        }

        //clipNodes
        const BSPChunk& clipNodesChunk = chunks[static_cast<size_t>(BSPChunk::Type::CLIP_NODES)];
        istream.seekg(clipNodesChunk.offset, std::ios_base::beg);
        unsigned long clipNodeCount = clipNodesChunk.length / sizeof(ClipNode);
        this->clipNodes.resize(clipNodeCount);

        for (ClipNode& clipNode : this->clipNodes) {
            Resources::IO::read(istream, clipNode.planeIndex);
            Resources::IO::read(istream, clipNode.childIndices[0]);
            Resources::IO::read(istream, clipNode.childIndices[1]);
        }

        //models
        const BSPChunk& modelsChunk = chunks[static_cast<size_t>(BSPChunk::Type::MODELS)];
        istream.seekg(modelsChunk.offset, std::ios_base::beg);
        unsigned long modelCount = modelsChunk.length / sizeof(Model);
        this->models.resize(modelCount);

        for (Model& model : this->models) {
            Resources::IO::read(istream, model.aabb.min.x);
            Resources::IO::read(istream, model.aabb.min.z);
            Resources::IO::read(istream, model.aabb.min.y);
            Resources::IO::read(istream, model.aabb.max.x);
            Resources::IO::read(istream, model.aabb.max.z);
            Resources::IO::read(istream, model.aabb.max.y);
            Resources::IO::read(istream, model.origin.x);
            Resources::IO::read(istream, model.origin.z);
            Resources::IO::read(istream, model.origin.y);
            Resources::IO::read(istream, model.headNodeIndices[0]);
            Resources::IO::read(istream, model.headNodeIndices[1]);
            Resources::IO::read(istream, model.headNodeIndices[2]);
            Resources::IO::read(istream, model.headNodeIndices[3]);
            Resources::IO::read(istream, model.visLeafs);
            Resources::IO::read(istream, model.faceStartIndex);
            Resources::IO::read(istream, model.faceCount);
            model.aabb.min.z = -model.aabb.min.z;
            model.aabb.max.z = -model.aabb.max.z;
            model.origin.z = -model.origin.z;
        }

        //visibility
        const BSPChunk& visibilityChunk = chunks[static_cast<size_t>(BSPChunk::Type::VISIBLIITY)];
        istream.seekg(visibilityChunk.offset, std::ios_base::beg);

        if (visibilityChunk.length > 0) {
            std::function<void(int)> countVisLeaves = [&](int node_index) {
                if (node_index < 0) {
                    if (node_index == -1 || this->leaves[~node_index].contentType == ContentType::SOLID) {
                        return;
                    }
                    ++this->visLeafCount;
                    return;
                }
                countVisLeaves(this->nodes[node_index].childIndices[0]);
                countVisLeaves(this->nodes[node_index].childIndices[1]);
            };

            countVisLeaves(0);
            std::vector<unsigned char> visibilityData;
            Resources::IO::read(istream, visibilityData, visibilityChunk.length);

            for (size_t i = 0; i < this->visLeafCount; ++i) {
                const Leaf& leaf = this->leaves[i + 1];
                if (leaf.visibilityOffset < 0) {
                    continue;
                }

                boost::dynamic_bitset<> leafPvs = boost::dynamic_bitset<>(leafCount - 1);
                leafPvs.reset();
                size_t leafPvsIndex = 0;
                std::__wrap_iter<unsigned char*> visibilityDataItr = visibilityData.begin() + leaf.visibilityOffset;

                while (leafPvsIndex < this->visLeafCount) {
                    if (*visibilityDataItr == 0) {
                        ++visibilityDataItr;
                        leafPvsIndex += 8 * (*visibilityDataItr);
                    } else {
                        for (unsigned char mask = 1; mask != 0; ++leafPvsIndex, mask <<= 1) {
                            if ((*visibilityDataItr & mask) && (leafPvsIndex < this->visLeafCount)) {
                                leafPvs[leafPvsIndex] = true;
                            }
                        }
                    }
                    ++visibilityDataItr;
                }
                this->leafPvsMap.insert(std::make_pair(i, std::move(leafPvs)));
            }
        }

        //textures
        const BSPChunk& texturesChunk = chunks[static_cast<size_t>(BSPChunk::Type::TEXTURES)];
        istream.seekg(texturesChunk.offset, std::ios_base::beg);
        unsigned int textureCount;
        Resources::IO::read(istream, textureCount);
        std::vector<unsigned int> textureOffsets;
        textureOffsets.resize(textureCount);

        for (unsigned int& textureOffset : textureOffsets) {
            Resources::IO::read(istream, textureOffset);
        }

        std::vector<BSPTexture> bspTextures;

        for (unsigned int i = 0; i < textureCount; ++i) {
            istream.seekg(texturesChunk.offset + textureOffsets[i], std::ios_base::beg);

            char textureNameBytes[16];
            Resources::IO::read(istream, textureNameBytes);
            std::string textureName = textureNameBytes;
            BSPTexture bspTexture{};

            Resources::IO::read(istream, bspTexture.width);
            Resources::IO::read(istream, bspTexture.height);
            Resources::IO::read(istream, bspTexture.mipmapOffsets);

            bspTextures.push_back(bspTexture);
            textureName.append(".png");
            boost::shared_ptr<Resources::Texture> texture;

            try {
                texture = Resources::resources.get<Resources::Texture>(textureName);
            } catch (...) {
                spdlog::error("Could not load texture: {}", textureName);
            }
            this->textures.push_back(texture);
        }

        //texture_info
        const BSPChunk& textureInfoChunk = chunks[static_cast<size_t>(BSPChunk::Type::TEXTURE_INFO)];
        istream.seekg(textureInfoChunk.offset, std::ios_base::beg);
        unsigned long textureInfoCount = textureInfoChunk.length / sizeof(TextureInfo);
        this->textureInfos.resize(textureInfoCount);

        for (TextureInfo& textureInfo : this->textureInfos) {
            Resources::IO::read(istream, textureInfo.s.axis.x);
            Resources::IO::read(istream, textureInfo.s.axis.z);
            Resources::IO::read(istream, textureInfo.s.axis.y);
            Resources::IO::read(istream, textureInfo.s.offset);
            Resources::IO::read(istream, textureInfo.t.axis.x);
            Resources::IO::read(istream, textureInfo.t.axis.z);
            Resources::IO::read(istream, textureInfo.t.axis.y);
            Resources::IO::read(istream, textureInfo.t.offset);
            Resources::IO::read(istream, textureInfo.textureIndex);
            Resources::IO::read(istream, textureInfo.flags);
            textureInfo.s.axis.z = -textureInfo.s.axis.z;
            textureInfo.t.axis.z = -textureInfo.t.axis.z;
        }

        std::vector<IndexType> indices;
        std::vector<VertexType> vertices;

        for (Face& face : this->faces) {
            auto normal = this->planes[face.planeIndex].plane.normal;
            if (face.planeSide != 0) {
                normal = -normal;
            }

            this->faceStartIndices.push_back(indices.size());
            TextureInfo& textureInfo = this->textureInfos[face.textureInfoIndex];
            BSPTexture& bspTexture = bspTextures[textureInfo.textureIndex];

            for (auto i = 0; i < face.surfaceEdgeCount; ++i) {
                VertexType vertex;
                int edgeIndex = this->surfaceEdges[face.surfaceEdgeStartIndex + i];
                if (edgeIndex > 0) {
                    vertex.location = vertexLocations[this->edges[edgeIndex].vertexIndices[0]];
                } else {
                    edgeIndex = -edgeIndex;
                    vertex.location = vertexLocations[this->edges[edgeIndex].vertexIndices[1]];
                }

                vertex.diffuseTexcoord.x = (glm::dot(vertex.location, textureInfo.s.axis) + textureInfo.s.offset) / bspTexture.width;
                vertex.diffuseTexcoord.y = -(glm::dot(vertex.location, textureInfo.t.axis) + textureInfo.t.offset) / bspTexture.height;
                vertices.push_back(vertex);
                indices.push_back(static_cast<unsigned int>(indices.size()));
            }
        }

        //lighting
        const BSPChunk& lightingChunk = chunks[static_cast<size_t>(BSPChunk::Type::LIGHTING)];
        istream.seekg(lightingChunk.offset, std::ios_base::beg);

        std::vector<unsigned char> lightingData;
        Resources::IO::read(istream, lightingData, lightingChunk.length);
        this->faceLightmapTextures.resize(this->faces.size());

        for (size_t faceIndex = 0; faceIndex < this->faces.size(); ++faceIndex) {
            Face& face = this->faces[faceIndex];
            if (face.lightingStyles[0] == 0 && static_cast<int>(face.lightmapOffset) >= -1) {
                float min_u = std::numeric_limits<float>::max();
                float min_v = std::numeric_limits<float>::max();
                float max_u = -std::numeric_limits<float>::max();
                float max_v = -std::numeric_limits<float>::max();

                TextureInfo& textureInfo = this->textureInfos[face.textureInfoIndex];

                for (int surfaceEdgeIndex = 0; surfaceEdgeIndex < face.surfaceEdgeCount; ++surfaceEdgeIndex) {
                    int edgeIndex = this->surfaceEdges[face.surfaceEdgeStartIndex + surfaceEdgeIndex];
                    glm::vec3 vertexLocation;

                    if (edgeIndex >= 0) {
                        vertexLocation = vertexLocations[this->edges[edgeIndex].vertexIndices[0]];
                    } else {
                        vertexLocation = vertexLocations[this->edges[-edgeIndex].vertexIndices[1]];
                    }

                    float u = glm::dot(textureInfo.s.axis, vertexLocation) + textureInfo.s.offset;
                    min_u = glm::min(u, min_u);
                    max_u = glm::max(u, max_u);

                    float v = glm::dot(textureInfo.t.axis, vertexLocation) + textureInfo.t.offset;
                    min_v = glm::min(v, min_v);
                    max_v = glm::max(v, max_v);
                }

                float textureMin_u = glm::floor(min_u / 16);
                float textureMin_v = glm::floor(min_v / 16);
                float textureMax_u = glm::ceil(max_u / 16);
                float textureMax_v = glm::ceil(max_v / 16);

                glm::vec2 textureSize;
                textureSize.x = textureMax_u - textureMin_u + 1;
                textureSize.y = textureMax_v - textureMin_v + 1;

                for (int surfaceEdgeIndex = 0; surfaceEdgeIndex < face.surfaceEdgeCount; ++surfaceEdgeIndex) {
                    int edgeIndex = this->surfaceEdges[face.surfaceEdgeStartIndex + surfaceEdgeIndex];
                    glm::vec3 vertexLocation;

                    if (edgeIndex >= 0) {
                        vertexLocation = vertexLocations[this->edges[edgeIndex].vertexIndices[0]];
                    } else {
                        vertexLocation = vertexLocations[this->edges[-edgeIndex].vertexIndices[1]];
                    }

                    float u = glm::dot(textureInfo.s.axis, vertexLocation) + textureInfo.s.offset;
                    float v = glm::dot(textureInfo.t.axis, vertexLocation) + textureInfo.t.offset;

                    float lightmap_u = (textureSize.x / 2) + (u - ((min_u + max_u) / 2)) / 16;
                    float lightmap_v = (textureSize.y / 2) + (v - ((min_v + max_v) / 2)) / 16;

                    glm::vec2& lightmapTexcoord = vertices[this->faceStartIndices[faceIndex] + surfaceEdgeIndex].lightmapTexcoord;
                    lightmapTexcoord.x = lightmap_u / textureSize.x;
                    lightmapTexcoord.y = lightmap_v / textureSize.y;
                }

                int lightingDataSize = 3 * static_cast<int>(textureSize.x) * static_cast<int>(textureSize.y);

                boost::shared_ptr<Resources::Image> image = boost::make_shared<Resources::Image>(
                        static_cast<Resources::Image::SizeType>(textureSize),
                        8,
                        Device::GPU::ColorType::RGB,
                        lightingData.data() + face.lightmapOffset,
                        lightingDataSize
                );

                boost::shared_ptr<Resources::Texture> lightmapTexture = boost::make_shared<Resources::Texture>(image);
                this->faceLightmapTextures[faceIndex] = lightmapTexture;
            }
        }

        //entities
        const BSPChunk& entitiesChunk = chunks[static_cast<size_t>(BSPChunk::Type::ENTITIES)];
        istream.seekg(entitiesChunk.offset, std::ios_base::beg);
        std::vector<char> entitiesBuffer;
        Resources::IO::read(istream, entitiesBuffer, entitiesChunk.length);
        std::string entitiesString = entitiesBuffer.data();
        size_t end = -1;

        for (;;) {
            unsigned long begin = entitiesString.find_first_of('{', end + 1);

            if (begin == -1) break;

            end = entitiesString.find_first_of('}', begin + 1);
            std::string entityString = entitiesString.substr(begin + 1, end - begin - 1);
            BSPEntity entity(entityString);
            boost::optional<std::string> modelOptional = entity.getOptional<std::string>("model");
            if (modelOptional) {
                std::string model = modelOptional.get();
                if (boost::algorithm::starts_with(model, "*")) {
                    this->brushEntityIndices.push_back(this->entities.size());
                }
            }
            this->entities.emplace_back(std::move(entity));
        }

        this->vertexBuffer = Device::GPU::Buffers::gpuBuffers.make<VertexBufferType>().lock();
        this->vertexBuffer->data(vertices, Device::GPU::Gpu::BufferUsage::STATIC_DRAW);
        this->indexBuffer = Device::GPU::Buffers::gpuBuffers.make<IndexBufferType>().lock();
        this->indexBuffer->data(indices, Device::GPU::Gpu::BufferUsage::STATIC_DRAW);
    }

    void BSP::render(const View::CameraParameters& cameraParameters) {
        boost::dynamic_bitset<> facesRendered = boost::dynamic_bitset<>(this->faces.size());
        facesRendered.reset();
        int cameraLeafIndex = getLeafIndexFromLocation(cameraParameters.location);

        //culling
        Device::GPU::Gpu::CullingStateManager::CullingState cullingState = Device::GPU::gpu.culling.getState();
        cullingState.isEnabled = true;
        cullingState.mode = Device::GPU::Gpu::CullingMode::FRONT;
        Device::GPU::gpu.culling.pushState(cullingState);

        //blend
        Device::GPU::Gpu::BlendStateManager::BlendState blendState = Device::GPU::gpu.blend.getState();
        blendState.isEnabled = false;
        Device::GPU::gpu.blend.pushState(blendState);

        static const auto DIFFUSE_TEXTURE_INDEX = 0;
        static const auto LIGHTMAP_TEXTURE_INDEX = 1;

        //bind buffers
        Device::GPU::gpu.buffers.push(Device::GPU::Gpu::BufferTarget::ARRAY, this->vertexBuffer);
        Device::GPU::gpu.buffers.push(Device::GPU::Gpu::BufferTarget::ELEMENT_ARRAY, this->indexBuffer);

        //bind program
        const boost::shared_ptr<BSPShader> gpuShader = Device::GPU::Shaders::shaders.get<BSPShader>();
        Device::GPU::gpu.programs.push(gpuShader);

        Device::GPU::gpu.setUniform("world_matrix", glm::mat4());
        Device::GPU::gpu.setUniform(
                "view_projection_matrix",
                cameraParameters.projectionMatrix * cameraParameters.viewMatrix
        );
        Device::GPU::gpu.setUniform("diffuse_texture", DIFFUSE_TEXTURE_INDEX);
        Device::GPU::gpu.setUniform("lightmap_texture", LIGHTMAP_TEXTURE_INDEX);
        Device::GPU::gpu.setUniform("lightmap_gamma", renderSettings.lightmap_gamma);

        auto renderFace = [&](int face_index) {
            if (facesRendered[face_index]) return;
            const Face& face = this->faces[face_index];
            if (face.lightingStyles[0] == Face::LIGHTING_STYLE_NONE) return;

            const boost::shared_ptr<Resources::Texture>& diffuseTexture = this->textures[this->textureInfos[face.textureInfoIndex].textureIndex];
            const boost::shared_ptr<Resources::Texture>& lightmapTexture = this->faceLightmapTextures[face_index];
            Device::GPU::gpu.textures.bind(DIFFUSE_TEXTURE_INDEX, diffuseTexture);
            Device::GPU::gpu.textures.bind(LIGHTMAP_TEXTURE_INDEX, lightmapTexture);

            Device::GPU::gpu.drawElements(
                    Device::GPU::Gpu::PrimitiveType::TRIANGLE_FAN,
                    face.surfaceEdgeCount,
                    IndexBufferType::DATA_TYPE,
                    this->faceStartIndices[face_index] * sizeof(IndexType)
            );

            facesRendered[face_index] = true;
            ++this->renderStats.faceCount;
        };

        auto renderLeaf = [&](NodeIndexType leaf_index) {
            const Leaf& leaf = this->leaves[leaf_index];
            for (int i = 0; i < leaf.markSurfaceCount; ++i) {
                renderFace(this->markSurfaces[leaf.markSurfaceStartIndex + i]);
            }
            ++this->renderStats.leafCount;
        };

        std::function<void(NodeIndexType, NodeIndexType)> renderNode = [&](NodeIndexType node_index, NodeIndexType camera_leaf_index) {
            if (node_index < 0) {
                if (node_index == -1) return;
                auto leafPvsMapItr = this->leafPvsMap.find(camera_leaf_index - 1);
                if (camera_leaf_index > 0 &&
                    leafPvsMapItr != this->leafPvsMap.end() &&
                    !this->leafPvsMap[camera_leaf_index - 1][~node_index - 1]) return;

                renderLeaf(~node_index);
                return;
            }

            const Node& node = this->nodes[node_index];
            const BSPPlane& plane = this->planes[node.planeIndex];
            float distance = 0;

            switch (plane.type) {
                case BSPPlane::Type::X:
                    distance = cameraParameters.location.x - plane.plane.distance;
                    break;
                case BSPPlane::Type::Y:
                    distance = cameraParameters.location.y - plane.plane.distance;
                    break;
                case BSPPlane::Type::Z:
                    distance = cameraParameters.location.z - plane.plane.distance;
                    break;
                default:
                    distance = glm::dot(plane.plane.normal, cameraParameters.location) - plane.plane.distance;
            }

            if (distance > 0) {
                renderNode(node.childIndices[1], camera_leaf_index);
                renderNode(node.childIndices[0], camera_leaf_index);
            } else {
                renderNode(node.childIndices[0], camera_leaf_index);
                renderNode(node.childIndices[1], camera_leaf_index);
            }
        };

        auto renderBrushEntity = [&](size_t entity_index) {
//            return;

            const BSPEntity& entity = this->entities[entity_index];
            const int modelIndex = boost::lexical_cast<int>(entity.get("model").substr(1));
            const Model& model = this->models[modelIndex];

            //render mode
            boost::optional<int> renderModeOptional = entity.getOptional<int>("rendermode");
            RenderMode renderMode = RenderMode::NORMAL;
            if (renderModeOptional) renderMode = static_cast<RenderMode>(renderModeOptional.get());

            //alpha
            float alpha = 1.0f;
            boost::optional<unsigned char> alphaOptional = entity.getOptional<unsigned char>("renderamt");
            if (alphaOptional) alpha = static_cast<float>(alphaOptional.get()) / 255.0f;

            //origin
            glm::vec3 origin;
            boost::optional<std::string> originOptional = entity.getOptional("origin");
            if (originOptional) {
                boost::is_any_of(" ");
                std::vector<std::string> tokens;

#if defined(_MSC_VER) && _MSC_VER >= 1400
                #pragma warning(push)
#pragma warning(disable:4996)
#endif
                boost::algorithm::split(tokens, originOptional.get(), boost::is_any_of(" "), boost::algorithm::token_compress_on);
#if defined(_MSC_VER) && _MSC_VER >= 1400
#pragma warning(pop)
#endif
                origin.x = boost::lexical_cast<float>(tokens[0]);
                origin.y = boost::lexical_cast<float>(tokens[2]);
                origin.z = -boost::lexical_cast<float>(tokens[1]);
            }

            //color
            glm::vec4 color(1.0f);
            boost::optional<std::string> colorOptional = entity.getOptional("rendercolor");
            if (colorOptional) {
                std::vector<std::string> tokens;
                boost::algorithm::split(tokens, colorOptional.get(), boost::is_any_of(" "), boost::algorithm::token_compress_on);

                color.r = boost::lexical_cast<float>(tokens[0]) / 255.0f;
                color.g = boost::lexical_cast<float>(tokens[1]) / 255.0f;
                color.b = boost::lexical_cast<float>(tokens[2]) / 255.0f;
            }

            Device::GPU::Gpu::BlendStateManager::BlendState _blendState = Device::GPU::gpu.blend.getState();
            Device::GPU::Gpu::Depth::State depthState = Device::GPU::gpu.depth.getState();
            depthState.shouldTest = true;
            const boost::shared_ptr<BSPShader> _gpuShader = Device::GPU::Shaders::shaders.get<BSPShader>();

            switch (renderMode) {
                case RenderMode::TEXTURE:
                    Device::GPU::gpu.setUniform("alpha", 0.0f);
                    _blendState.isEnabled = true;
                    _blendState.srcFactor = Device::GPU::Gpu::BlendFactor::SRC_ALPHA;
                    _blendState.dstFactor = Device::GPU::Gpu::BlendFactor::ONE;
                    break;
                case RenderMode::SOLID:
                    Device::GPU::gpu.setUniform("should_test_alpha", 1);
                    break;
                case RenderMode::ADDITIVE:
                    Device::GPU::gpu.setUniform("alpha", alpha);
                    _blendState.isEnabled = true;
                    _blendState.srcFactor = Device::GPU::Gpu::BlendFactor::ONE;
                    _blendState.dstFactor = Device::GPU::Gpu::BlendFactor::ONE;
                    depthState.shouldWriteMask = false;
                    break;
                default:
                    _blendState.isEnabled = false;
                    depthState.shouldWriteMask = true;
                    break;
            }

            Device::GPU::gpu.blend.pushState(_blendState);
            Device::GPU::gpu.depth.pushState(depthState);

            glm::mat4 world_matrix = glm::translate(glm::mat4x4(), model.origin);
            world_matrix *= glm::translate(glm::mat4x4(), origin);
            Device::GPU::gpu.setUniform("world_matrix", world_matrix);
            renderNode(model.headNodeIndices[0], -1);

            switch (renderMode) {
                case RenderMode::TEXTURE:
                case RenderMode::ADDITIVE:
                    Device::GPU::gpu.setUniform("alpha", 1.0f);
                    break;
                case RenderMode::SOLID:
                    Device::GPU::gpu.setUniform("should_test_alpha", 0);
                    break;
                default:
                    break;
            }

            Device::GPU::gpu.depth.popState();
            Device::GPU::gpu.blend.popState();
        };

        this->renderStats.reset();

        //Depth
        Device::GPU::Gpu::Depth::State depthState = Device::GPU::gpu.depth.getState();
        depthState.shouldTest = true;
        Device::GPU::gpu.depth.pushState(depthState);
        renderNode(0, cameraLeafIndex);
        Device::GPU::gpu.depth.popState();

        for (unsigned long brushEntityIndex : this->brushEntityIndices) {
            renderBrushEntity(brushEntityIndex);
        }

        Device::GPU::gpu.programs.pop();
        Device::GPU::gpu.buffers.pop(Device::GPU::Gpu::BufferTarget::ELEMENT_ARRAY);
        Device::GPU::gpu.buffers.pop(Device::GPU::Gpu::BufferTarget::ARRAY);
        Device::GPU::gpu.culling.popState();
        Device::GPU::gpu.blend.popState();
    }

    int BSP::getLeafIndexFromLocation(const glm::vec3& location) const {
        NodeIndexType nodeIndex = 0;

        while (nodeIndex >= 0) {
            const Node& node = this->nodes[nodeIndex];
            const ::Scenes::Structure::Plane3<float>& plane = this->planes[node.planeIndex].plane;

            if (glm::dot(plane.normal, (location - plane.origin())) >= 0) {
                nodeIndex = node.childIndices[0];
            } else {
                nodeIndex = node.childIndices[1];
            }
        }

        return ~nodeIndex;
    }
}