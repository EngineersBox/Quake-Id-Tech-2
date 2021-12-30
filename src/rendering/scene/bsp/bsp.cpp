#include <iostream>
#include <glm/ext.hpp>
#include <boost/algorithm/string.hpp>
#include <exception>
#include <spdlog/spdlog.h>

#include "bsp.hpp"
#include "cameraParams.hpp"
#include "image.hpp"
#include "gpu.hpp"
#include "collision.hpp"
#include "bspGpuProgram.hpp"
#include "resourceManager.hpp"
#include "gpuProgramManager.hpp"
#include "gpuBufferManager.hpp"
#include "../../../resources/io/io.hpp"
#include "cameraParams.hpp"


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
        const BSPChunk& plane_chunk = chunks[static_cast<size_t>(BSPChunk::Type::PLANES)];
        istream.seekg(plane_chunk.offset, std::ios_base::beg);

        unsigned long plane_count = plane_chunk.length / sizeof(BSPPlane);
        planes.resize(plane_count);

        for (BSPPlane& plane : planes) {
            Resources::IO::read(istream, plane.plane.normal.x);
            Resources::IO::read(istream, plane.plane.normal.z);
            Resources::IO::read(istream, plane.plane.normal.y);
            Resources::IO::read(istream, plane.plane.distance);
            Resources::IO::read(istream, plane.type);
            plane.plane.normal.z = -plane.plane.normal.z;
        }

        //vertex_locations
        const BSPChunk& vertices_chunk = chunks[static_cast<size_t>(BSPChunk::Type::VERTICES)];
        istream.seekg(vertices_chunk.offset, std::ios_base::beg);

        std::vector<glm::vec3> vertex_locations;
        unsigned long vertex_location_count = vertices_chunk.length / sizeof(glm::vec3);
        vertex_locations.resize(vertex_location_count);

        for (glm::vec<3, float>& vertex_location : vertex_locations) {
            Resources::IO::read(istream, vertex_location.x);
            Resources::IO::read(istream, vertex_location.z);
            Resources::IO::read(istream, vertex_location.y);
            vertex_location.z = -vertex_location.z;
        }

        //edges
        const BSPChunk& edges_chunk = chunks[static_cast<size_t>(BSPChunk::Type::EDGES)];
        istream.seekg(edges_chunk.offset, std::ios_base::beg);
        unsigned long edge_count = edges_chunk.length / sizeof(Edge);
        edges.resize(edge_count);

        for (Edge& edge : edges) {
            Resources::IO::read(istream, edge.vertex_indices[0]);
            Resources::IO::read(istream, edge.vertex_indices[1]);
        }

        //surface_edges
        const BSPChunk& surface_edges_chunk = chunks[static_cast<size_t>(BSPChunk::Type::SURFACE_EDGES)];
        istream.seekg(surface_edges_chunk.offset, std::ios_base::beg);
        unsigned long surface_edge_count = surface_edges_chunk.length / sizeof(int);
        surface_edges.resize(surface_edge_count);

        for (int& surface_edge : surface_edges) {
            Resources::IO::read(istream, surface_edge);
        }

        //faces
        const BSPChunk& faces_chunk = chunks[static_cast<size_t>(BSPChunk::Type::FACES)];
        istream.seekg(faces_chunk.offset, std::ios_base::beg);
        unsigned long face_count = faces_chunk.length / sizeof(Face);
        faces.resize(face_count);

        for (Face& face : faces) {
            Resources::IO::read(istream, face.plane_index);
            Resources::IO::read(istream, face.plane_side);
            Resources::IO::read(istream, face.surface_edge_start_index);
            Resources::IO::read(istream, face.surface_edge_count);
            Resources::IO::read(istream, face.texture_info_index);
            Resources::IO::read(istream, face.lighting_styles[0]);
            Resources::IO::read(istream, face.lighting_styles[1]);
            Resources::IO::read(istream, face.lighting_styles[2]);
            Resources::IO::read(istream, face.lighting_styles[3]);
            Resources::IO::read(istream, face.lightmap_offset);
        }

        //nodes
        const BSPChunk& nodes_chunk = chunks[static_cast<size_t>(BSPChunk::Type::NODES)];
        istream.seekg(nodes_chunk.offset, std::ios_base::beg);
        unsigned long node_count = nodes_chunk.length / sizeof(Node);
        nodes.resize(node_count);

        for (Node& node : nodes) {
            Resources::IO::read(istream, node.plane_index);
            Resources::IO::read(istream, node.child_indices[0]);
            Resources::IO::read(istream, node.child_indices[1]);
            Resources::IO::read(istream, node.aabb.min.x);
            Resources::IO::read(istream, node.aabb.min.z);
            Resources::IO::read(istream, node.aabb.min.y);
            Resources::IO::read(istream, node.aabb.max.x);
            Resources::IO::read(istream, node.aabb.max.z);
            Resources::IO::read(istream, node.aabb.max.y);
            Resources::IO::read(istream, node.face_start_index);
            Resources::IO::read(istream, node.face_count);
            node.aabb.min.z = -node.aabb.min.z;
            node.aabb.max.z = -node.aabb.max.z;
        }

        //leaves
        const BSPChunk& leaves_chunk = chunks[static_cast<size_t>(BSPChunk::Type::LEAVES)];
        istream.seekg(leaves_chunk.offset, std::ios_base::beg);
        unsigned long leaf_count = leaves_chunk.length / sizeof(Leaf);
        leafs.resize(leaf_count);

        for (auto& leaf : leafs) {
            Resources::IO::read(istream, leaf.content_type);
            Resources::IO::read(istream, leaf.visibility_offset);
            Resources::IO::read(istream, leaf.aabb.min.x);
            Resources::IO::read(istream, leaf.aabb.min.z);
            Resources::IO::read(istream, leaf.aabb.min.y);
            Resources::IO::read(istream, leaf.aabb.max.x);
            Resources::IO::read(istream, leaf.aabb.max.z);
            Resources::IO::read(istream, leaf.aabb.max.y);
            Resources::IO::read(istream, leaf.mark_surface_start_index);
            Resources::IO::read(istream, leaf.mark_surface_count);
            Resources::IO::read(istream, leaf.ambient_sound_levels[0]);
            Resources::IO::read(istream, leaf.ambient_sound_levels[1]);
            Resources::IO::read(istream, leaf.ambient_sound_levels[2]);
            Resources::IO::read(istream, leaf.ambient_sound_levels[3]);

            leaf.aabb.min.z = -leaf.aabb.min.z;
            leaf.aabb.max.z = -leaf.aabb.max.z;
        }

        //mark_surfaces
        const BSPChunk& mark_surfaces_chunk = chunks[static_cast<size_t>(BSPChunk::Type::MARK_SURFACES)];
        istream.seekg(mark_surfaces_chunk.offset, std::ios_base::beg);
        unsigned long mark_surface_count = mark_surfaces_chunk.length / sizeof(unsigned short);
        mark_surfaces.resize(mark_surface_count);

        for (unsigned short& mark_surface : mark_surfaces) {
            Resources::IO::read(istream, mark_surface);
        }

        //clip_nodes
        const BSPChunk& clip_nodes_chunk = chunks[static_cast<size_t>(BSPChunk::Type::CLIP_NODES)];
        istream.seekg(clip_nodes_chunk.offset, std::ios_base::beg);
        unsigned long clip_node_count = clip_nodes_chunk.length / sizeof(ClipNode);
        clip_nodes.resize(clip_node_count);

        for (ClipNode& clip_node : clip_nodes) {
            Resources::IO::read(istream, clip_node.plane_index);
            Resources::IO::read(istream, clip_node.child_indices[0]);
            Resources::IO::read(istream, clip_node.child_indices[1]);
        }

        //models
        const BSPChunk& models_chunk = chunks[static_cast<size_t>(BSPChunk::Type::MODELS)];
        istream.seekg(models_chunk.offset, std::ios_base::beg);
        unsigned long model_count = models_chunk.length / sizeof(Model);
        models.resize(model_count);

        for (Model& model : models) {
            Resources::IO::read(istream, model.aabb.min.x);
            Resources::IO::read(istream, model.aabb.min.z);
            Resources::IO::read(istream, model.aabb.min.y);
            Resources::IO::read(istream, model.aabb.max.x);
            Resources::IO::read(istream, model.aabb.max.z);
            Resources::IO::read(istream, model.aabb.max.y);
            Resources::IO::read(istream, model.origin.x);
            Resources::IO::read(istream, model.origin.z);
            Resources::IO::read(istream, model.origin.y);
            Resources::IO::read(istream, model.head_node_indices[0]);
            Resources::IO::read(istream, model.head_node_indices[1]);
            Resources::IO::read(istream, model.head_node_indices[2]);
            Resources::IO::read(istream, model.head_node_indices[3]);
            Resources::IO::read(istream, model.vis_leafs);
            Resources::IO::read(istream, model.face_start_index);
            Resources::IO::read(istream, model.face_count);
            model.aabb.min.z = -model.aabb.min.z;
            model.aabb.max.z = -model.aabb.max.z;
            model.origin.z = -model.origin.z;
        }

        //visibility
        const BSPChunk& visibility_chunk = chunks[static_cast<size_t>(BSPChunk::Type::VISIBLIITY)];
        istream.seekg(visibility_chunk.offset, std::ios_base::beg);

        if (visibility_chunk.length > 0) {
            std::function<void(int)> count_vis_leaves = [&](int node_index) {
                if (node_index < 0) {
                    if (node_index == -1 || leafs[~node_index].content_type == ContentType::SOLID) {
                        return;
                    }
                    ++vis_leaf_count;
                    return;
                }
                count_vis_leaves(nodes[node_index].child_indices[0]);
                count_vis_leaves(nodes[node_index].child_indices[1]);
            };

            count_vis_leaves(0);
            std::vector<unsigned char> visibility_data;
            Resources::IO::read(istream, visibility_data, visibility_chunk.length);

            for (size_t i = 0; i < vis_leaf_count; ++i) {
                const Leaf& leaf = leafs[i + 1];
                if (leaf.visibility_offset < 0) {
                    continue;
                }

                boost::dynamic_bitset<> leaf_pvs = boost::dynamic_bitset<>(leaf_count - 1);
                leaf_pvs.reset();
                size_t leaf_pvs_index = 0;
                std::__wrap_iter<unsigned char*> visibility_data_itr = visibility_data.begin() + leaf.visibility_offset;

                while (leaf_pvs_index < vis_leaf_count) {
                    if (*visibility_data_itr == 0) {
                        ++visibility_data_itr;
                        leaf_pvs_index += 8 * (*visibility_data_itr);
                    } else {
                        for (unsigned char mask = 1; mask != 0; ++leaf_pvs_index, mask <<= 1) {
                            if ((*visibility_data_itr & mask) && (leaf_pvs_index < vis_leaf_count)) {
                                leaf_pvs[leaf_pvs_index] = true;
                            }
                        }
                    }
                    ++visibility_data_itr;
                }
                leaf_pvs_map.insert(std::make_pair(i, std::move(leaf_pvs)));
            }
        }

        //textures
        const BSPChunk& textures_chunk = chunks[static_cast<size_t>(BSPChunk::Type::TEXTURES)];
        istream.seekg(textures_chunk.offset, std::ios_base::beg);
        unsigned int texture_count;
        Resources::IO::read(istream, texture_count);
        std::vector<unsigned int> texture_offsets;
        texture_offsets.resize(texture_count);

        for (unsigned int& texture_offset : texture_offsets) {
            Resources::IO::read(istream, texture_offset);
        }

        std::vector<BSPTexture> bsp_textures;

        for (unsigned int i = 0; i < texture_count; ++i) {
            istream.seekg(textures_chunk.offset + texture_offsets[i], std::ios_base::beg);

            char texture_name_bytes[16];
            Resources::IO::read(istream, texture_name_bytes);
            std::string texture_name = texture_name_bytes;
            BSPTexture bsp_texture;

            Resources::IO::read(istream, bsp_texture.width);
            Resources::IO::read(istream, bsp_texture.height);
            Resources::IO::read(istream, bsp_texture.mipmap_offsets);

            bsp_textures.push_back(bsp_texture);
            texture_name.append(".png");
            boost::shared_ptr<Texture> texture;

            try{
                texture = resources.get<Texture>(texture_name);
            } catch (...) {
                spdlog::error("Couldn't load texture: {}", texture_name);
            }
            textures.push_back(texture);
        }

        //texture_info
        const BSPChunk& texture_info_chunk = chunks[static_cast<size_t>(BSPChunk::Type::TEXTURE_INFO)];
        istream.seekg(texture_info_chunk.offset, std::ios_base::beg);
        unsigned long texture_info_count = texture_info_chunk.length / sizeof(TextureInfo);
        texture_infos.resize(texture_info_count);

        for (TextureInfo& texture_info : texture_infos) {
            Resources::IO::read(istream, texture_info.s.axis.x);
            Resources::IO::read(istream, texture_info.s.axis.z);
            Resources::IO::read(istream, texture_info.s.axis.y);
            Resources::IO::read(istream, texture_info.s.offset);
            Resources::IO::read(istream, texture_info.t.axis.x);
            Resources::IO::read(istream, texture_info.t.axis.z);
            Resources::IO::read(istream, texture_info.t.axis.y);
            Resources::IO::read(istream, texture_info.t.offset);
            Resources::IO::read(istream, texture_info.texture_index);
            Resources::IO::read(istream, texture_info.flags);
            texture_info.s.axis.z = -texture_info.s.axis.z;
            texture_info.t.axis.z = -texture_info.t.axis.z;
        }

        std::vector<IndexType> indices;
        std::vector<VertexType> vertices;

        for (Face& face : faces) {
            auto normal = planes[face.plane_index].plane.normal;
            if (face.plane_side != 0) {
                normal = -normal;
            }

            face_start_indices.push_back(indices.size());
            TextureInfo& texture_info = texture_infos[face.texture_info_index];
            BSPTexture& bsp_texture = bsp_textures[texture_info.texture_index];

            for (auto i = 0; i < face.surface_edge_count; ++i) {
                VertexType vertex;
                int edge_index = surface_edges[face.surface_edge_start_index + i];
                if (edge_index > 0) {
                    vertex.location = vertex_locations[edges[edge_index].vertex_indices[0]];
                } else {
                    edge_index = -edge_index;
                    vertex.location = vertex_locations[edges[edge_index].vertex_indices[1]];
                }

                vertex.diffuse_texcoord.x = (glm::dot(vertex.location, texture_info.s.axis) + texture_info.s.offset) / bsp_texture.width;
                vertex.diffuse_texcoord.y = -(glm::dot(vertex.location, texture_info.t.axis) + texture_info.t.offset) / bsp_texture.height;
                vertices.push_back(vertex);
                indices.push_back(static_cast<unsigned int>(indices.size()));
            }
        }

        //lighting
        const BSPChunk& lighting_chunk = chunks[static_cast<size_t>(BSPChunk::Type::LIGHTING)];
        istream.seekg(lighting_chunk.offset, std::ios_base::beg);

        std::vector<unsigned char> lighting_data;
        Resources::IO::read(istream, lighting_data, lighting_chunk.length);
        face_lightmap_textures.resize(faces.size());

        for (size_t face_index = 0; face_index < faces.size(); ++face_index) {
            Face& face = faces[face_index];

            if (face.lighting_styles[0] == 0 && static_cast<int>(face.lightmap_offset) >= -1) {
                float min_u = std::numeric_limits<float>::max();
                float min_v = std::numeric_limits<float>::max();
                float max_u = -std::numeric_limits<float>::max();
                float max_v = -std::numeric_limits<float>::max();

                TextureInfo& texture_info = texture_infos[face.texture_info_index];

                for (int surface_edge_index = 0; surface_edge_index < face.surface_edge_count; ++surface_edge_index) {
                    int edge_index = surface_edges[face.surface_edge_start_index + surface_edge_index];
                    glm::vec3 vertex_location;

                    if (edge_index >= 0) {
                        vertex_location = vertex_locations[edges[edge_index].vertex_indices[0]];
                    } else {
                        vertex_location = vertex_locations[edges[-edge_index].vertex_indices[1]];
                    }

                    float u = glm::dot(texture_info.s.axis, vertex_location) + texture_info.s.offset;
                    min_u = glm::min(u, min_u);
                    max_u = glm::max(u, max_u);

                    float v = glm::dot(texture_info.t.axis, vertex_location) + texture_info.t.offset;
                    min_v = glm::min(v, min_v);
                    max_v = glm::max(v, max_v);
                }

                float texture_min_u = glm::floor(min_u / 16);
                float texture_min_v = glm::floor(min_v / 16);
                float texture_max_u = glm::ceil(max_u / 16);
                float texture_max_v = glm::ceil(max_v / 16);

                glm::vec2 texture_size;
                texture_size.x = texture_max_u - texture_min_u + 1;
                texture_size.y = texture_max_v - texture_min_v + 1;

                for (int surface_edge_index = 0; surface_edge_index < face.surface_edge_count; ++surface_edge_index) {
                    int edge_index = surface_edges[face.surface_edge_start_index + surface_edge_index];
                    glm::vec3 vertex_location;

                    if (edge_index >= 0) {
                        vertex_location = vertex_locations[edges[edge_index].vertex_indices[0]];
                    } else {
                        vertex_location = vertex_locations[edges[-edge_index].vertex_indices[1]];
                    }

                    float u = glm::dot(texture_info.s.axis, vertex_location) + texture_info.s.offset;
                    float v = glm::dot(texture_info.t.axis, vertex_location) + texture_info.t.offset;

                    float lightmap_u = (texture_size.x / 2) + (u - ((min_u + max_u) / 2)) / 16;
                    float lightmap_v = (texture_size.y / 2) + (v - ((min_v + max_v) / 2)) / 16;

                    auto& lightmap_texcoord = vertices[face_start_indices[face_index] + surface_edge_index].lightmap_texcoord;
                    lightmap_texcoord.x = lightmap_u / texture_size.x;
                    lightmap_texcoord.y = lightmap_v / texture_size.y;
                }

                auto lighting_data_size = 3 * static_cast<int>(texture_size.x) * static_cast<int>(texture_size.y);

                auto image = boost::make_shared<Image>(
                        static_cast<Image::SizeType>(texture_size),
                        8,
                        ColorType::RGB,
                        lighting_data.data() + face.lightmap_offset,
                        lighting_data_size);

                auto lightmap_texture = boost::make_shared<Texture>(image);

                face_lightmap_textures[face_index] = lightmap_texture;
            }
        }

        //entities
        const BSPChunk& entities_chunk = chunks[static_cast<size_t>(BSPChunk::Type::ENTITIES)];
        istream.seekg(entities_chunk.offset, std::ios_base::beg);
        std::vector<char> entities_buffer;
        Resources::IO::read(istream, entities_buffer, entities_chunk.length);
        std::string entities_string = entities_buffer.data();
        size_t end = -1;

        for (;;) {
            unsigned long begin = entities_string.find_first_of('{', end + 1);

            if (begin == -1) break;

            end = entities_string.find_first_of('}', begin + 1);
            std::string entity_string = entities_string.substr(begin + 1, end - begin - 1);
            BSPEntity entity(entity_string);
            auto model_optional = entity.get_optional<std::string>("model");
            if (model_optional) {
                auto model = model_optional.get();
                if (boost::algorithm::starts_with(model, "*")) {
                    brush_entity_indices.push_back(entities.size());
                }
            }
            entities.emplace_back(std::move(entity));
        }

        vertex_buffer = gpu_buffers.make<VertexBufferType>().lock();
        vertex_buffer->data(vertices, Gpu::BufferUsage::STATIC_DRAW);
        index_buffer = gpu_buffers.make<IndexBufferType>().lock();
        index_buffer->data(indices, Gpu::BufferUsage::STATIC_DRAW);
    }

    void BSP::render(const CameraParameters& camera_parameters) {
        boost::dynamic_bitset<> faces_rendered = boost::dynamic_bitset<>(faces.size());
        faces_rendered.reset();
        auto camera_leaf_index = get_leaf_index_from_location(camera_parameters.location);

        //culling
        auto culling_state = gpu.culling.get_state();
        culling_state.is_enabled = true;
        culling_state.mode = Gpu::CullingMode::FRONT;
        gpu.culling.push_state(culling_state);

        //blend
        auto blend_state = gpu.blend.get_state();
        blend_state.is_enabled = false;
        gpu.blend.push_state(blend_state);

        static const auto DIFFUSE_TEXTURE_INDEX = 0;
        static const auto LIGHTMAP_TEXTURE_INDEX = 1;

        //bind buffers
        gpu.buffers.push(Gpu::BufferTarget::ARRAY, vertex_buffer);
        gpu.buffers.push(Gpu::BufferTarget::ELEMENT_ARRAY, index_buffer);

        //bind program
        const auto gpu_program = gpu_programs.get<bsp_gpu_program>();
        gpu.programs.push(gpu_program);

        gpu.set_uniform("world_matrix", glm::mat4());
        gpu.set_uniform("view_projection_matrix", camera_parameters.projection_matrix * camera_parameters.view_matrix);
        gpu.set_uniform("diffuse_texture", DIFFUSE_TEXTURE_INDEX);
        gpu.set_uniform("lightmap_texture", LIGHTMAP_TEXTURE_INDEX);
        gpu.set_uniform("lightmap_gamma", render_settings.lightmap_gamma);

        auto render_face = [&](int face_index) {
            if (faces_rendered[face_index]) return;
            const Face& face = faces[face_index];
            if (face.lighting_styles[0] == Face::LIGHTING_STYLE_NONE) return;

            const auto& diffuse_texture = textures[texture_infos[face.texture_info_index].texture_index];
            const auto& lightmap_texture = face_lightmap_textures[face_index];
            gpu.textures.bind(DIFFUSE_TEXTURE_INDEX, diffuse_texture);
            gpu.textures.bind(LIGHTMAP_TEXTURE_INDEX, lightmap_texture);

            gpu.draw_elements(Gpu::PrimitiveType::TRIANGLE_FAN,
                              face.surface_edge_count,
                              IndexBufferType::DATA_TYPE,
                              face_start_indices[face_index] * sizeof(IndexType));

            faces_rendered[face_index] = true;
            ++render_stats.face_count;
        };

        auto render_leaf = [&](NodeIndexType leaf_index) {
            const Leaf& leaf = leafs[leaf_index];
            for (int i = 0; i < leaf.mark_surface_count; ++i) {
                render_face(mark_surfaces[leaf.mark_surface_start_index + i]);
            }
            ++render_stats.leaf_count;
        };

        std::function<void(NodeIndexType, NodeIndexType)> render_node = [&](NodeIndexType node_index, NodeIndexType camera_leaf_index) {
            if (node_index < 0) {
                if (node_index == -1) return
                auto leaf_pvs_map_itr = leaf_pvs_map.find(camera_leaf_index - 1);
                if (camera_leaf_index > 0 &&
                    leaf_pvs_map_itr != leaf_pvs_map.end() &&
                    !leaf_pvs_map[camera_leaf_index - 1][~node_index - 1]) return;

                render_leaf(~node_index);
                return;
            }

            const Node& node = nodes[node_index];
            const BSPPlane& plane = planes[node.plane_index];
            float distance = 0;

            switch (plane.type) {
                case BSPPlane::Type::X:
                    distance = camera_parameters.location.x - plane.plane.distance;
                    break;
                case BSPPlane::Type::Y:
                    distance = camera_parameters.location.y - plane.plane.distance;
                    break;
                case BSPPlane::Type::Z:
                    distance = camera_parameters.location.z - plane.plane.distance;
                    break;
                default:
                    distance = glm::dot(plane.plane.normal, camera_parameters.location) - plane.plane.distance;
            }

            if (distance > 0) {
                render_node(node.child_indices[1], camera_leaf_index);
                render_node(node.child_indices[0], camera_leaf_index);
            } else {
                render_node(node.child_indices[0], camera_leaf_index);
                render_node(node.child_indices[1], camera_leaf_index);
            }
        };

        auto render_brush_entity = [&](size_t entity_index) {
            return;

            const auto& entity = entities[entity_index];
            const auto model_index = boost::lexical_cast<int>(entity.get("model").substr(1));
            const auto& model = models[model_index];

            //render mode
            auto render_mode_optional = entity.get_optional<int>("rendermode");
            auto render_mode = RenderMode::NORMAL;
            if (render_mode_optional) render_mode = static_cast<RenderMode>(render_mode_optional.get());

            //alpha
            float alpha = 1.0f;
            auto alpha_optional = entity.get_optional<unsigned char>("renderamt");
            if (alpha_optional) alpha = static_cast<float>(alpha_optional.get()) / 255.0f;

            //origin
            glm::vec3 origin;
            auto origin_optional = entity.get_optional("origin");
            if (origin_optional) {
                boost::is_any_of(" ");
                std::vector<std::string> tokens;

#if defined(_MSC_VER) && _MSC_VER >= 1400
                #pragma warning(push)
#pragma warning(disable:4996) 
#endif
                boost::algorithm::split(tokens, origin_optional.get(), boost::is_any_of(" "), boost::algorithm::token_compress_on);
#if defined(_MSC_VER) && _MSC_VER >= 1400
#pragma warning(pop) 
#endif
                origin.x = boost::lexical_cast<float>(tokens[0]);
                origin.y = boost::lexical_cast<float>(tokens[2]);
                origin.z = -boost::lexical_cast<float>(tokens[1]);
            }

            //color
            glm::vec4 color(1.0f);
            auto color_optional = entity.get_optional("rendercolor");
            if (color_optional) {
                std::vector<std::string> tokens;
                boost::algorithm::split(tokens, color_optional.get(), boost::is_any_of(" "), boost::algorithm::token_compress_on);

                color.r = boost::lexical_cast<float>(tokens[0]) / 255.0f;
                color.g = boost::lexical_cast<float>(tokens[1]) / 255.0f;
                color.b = boost::lexical_cast<float>(tokens[2]) / 255.0f;
            }

            auto blend_state = gpu.blend.get_state();
            auto depth_state = gpu.depth.get_state();
            depth_state.should_test = true;
            const auto gpu_program = gpu_programs.get<bsp_gpu_program>();

            switch (render_mode) {
                case RenderMode::TEXTURE:
                    gpu.set_uniform("alpha", 0.0f);
                    blend_state.is_enabled = true;
                    blend_state.src_factor = Gpu::BlendFactor::SRC_ALPHA;
                    blend_state.dst_factor = Gpu::BlendFactor::ONE;
                    break;
                case RenderMode::SOLID:
                    gpu.set_uniform("should_test_alpha", 1);
                    break;
                case RenderMode::ADDITIVE:
                    gpu.set_uniform("alpha", alpha);
                    blend_state.is_enabled = true;
                    blend_state.src_factor = Gpu::BlendFactor::ONE;
                    blend_state.dst_factor = Gpu::BlendFactor::ONE;
                    depth_state.should_write_mask = false;
                    break;
                default:
                    blend_state.is_enabled = false;
                    depth_state.should_write_mask = true;
                    break;
            }

            gpu.blend.push_state(blend_state);
            gpu.depth.push_state(depth_state);
            auto world_matrix = ::glm::translate(model.origin);
            world_matrix *= ::glm::translate(origin);
            gpu.set_uniform("world_matrix", world_matrix);
            render_node(model.head_node_indices[0], -1);

            switch (render_mode) {
                case RenderMode::TEXTURE:
                case RenderMode::ADDITIVE:
                    gpu.set_uniform("alpha", 1.0f);
                    break;
                case RenderMode::SOLID:
                    gpu.set_uniform("should_test_alpha", 0);
                    break;
                default:
                    break;
            }

            gpu.depth.pop_state();
            gpu.blend.pop_state();
        };

        render_stats.reset();

        //depth
        auto depth_state = gpu.depth.get_state();
        depth_state.should_test = true;
        gpu.depth.push_state(depth_state);
        render_node(0, camera_leaf_index);
        gpu.depth.pop_state();

        for (unsigned long brush_entity_index : brush_entity_indices) {
            render_brush_entity(brush_entity_index);
        }

        gpu.programs.pop();
        gpu.buffers.pop(Gpu::BufferTarget::ELEMENT_ARRAY);
        gpu.buffers.pop(Gpu::BufferTarget::ARRAY);
        gpu.culling.pop_state();
        gpu.blend.pop_state();
    }

    int BSP::get_leaf_index_from_location(const glm::vec3& location) const {
        NodeIndexType node_index = 0;

        while (node_index >= 0) {
            const Node& node = nodes[node_index];
            const ::Scene::Structure::Plane3<float>& plane = planes[node.plane_index].plane;

            if (glm::dot(plane.normal, (location - plane.origin())) >= 0) {
                node_index = node.child_indices[0];
            } else {
                node_index = node.child_indices[1];
            }
        }

        return ~node_index;
    }
}