//
//  Square
//
//  Created by Gabriele Di Bari on 18/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include "MeshManager.h"
#include "GLTFImport.h"
#include <cstddef>

MeshManager::MeshManager(Square::Context& context, const std::string& output, unsigned char mode)
: m_context(context)
, m_output(output)
, m_mode(mode)
{}

MeshManager::MeshManager(Square::Context& context, const std::string& output, unsigned char mode, const Square::Data::GLTF::GLTF& gltf)
: m_context(context)
, m_output(output)
, m_mode(mode)
{
    for (auto& mesh : gltf.meshes)
    {
        add_mesh(mesh, gltf);
    }
}

const std::unordered_map<std::string, std::string>& MeshManager::resource_map() const
{
    return m_mesh_name_files;
}

std::optional< std::tuple<const std::string*, const Square::Geometry::OBoundingBox*, const std::vector<size_t>* > > MeshManager::at(size_t id) const
{
    if (id < m_mesh_names.size() && id < m_mesh_obbs.size())
    {
        return { std::make_tuple(&m_mesh_names[id], &m_mesh_obbs[id], &m_meshes_materials[id]) };
    }
    return {};
}

size_t MeshManager::add_mesh(const Square::Data::GLTF::Mesh& mesh, const Square::Data::GLTF::GLTF& gltf)
{
    using namespace Square;
    using namespace Square::Data;
    using namespace Square::Scene;
    //push material list
    m_meshes_materials.push_back({});
    auto& mesh_materials_ids = m_meshes_materials.back();
    //build context
    Parser::StaticMesh::Context static_mesh_context;
    // Mesh geometry 
    Render::Mesh::Vertex3DNTBUVList context_mesh;
    Render::Mesh::IndexList context_index;
    // The joints of the vertices and their weights (a skinned mesh: a primitive with them)
    std::vector<Vec4> context_joints;
    std::vector<Vec4> context_weights;
    bool skinned = false;
    // Save
    size_t primitive_id = 0;
    for (const auto& primitive : mesh.primitives)
    {
        switch (GLTF::Import::determine_structure(primitive.attributes, gltf.accessors))
        {
        case GLTF::Import::StructureType::Position2D:
        case GLTF::Import::StructureType::Position2DUV:
        case GLTF::Import::StructureType::Position3D:
        case GLTF::Import::StructureType::Position3DUV:
        case GLTF::Import::StructureType::Position3DNormalUV:
        case GLTF::Import::StructureType::Position3DNormalTangentBinormalUV:
        {
            // Get draw type
            auto drawtype = GLTF::Import::get_DrawType(primitive);
            // Invalid draw type?
            if (drawtype == Render::DrawType::DRAW_INVALID)
            {
                m_context.logger()->warning("Error to import primitive[" + std::to_string(primitive_id) + "] mesh: " + mesh.name);
                continue;
            }
            // Get data
            auto indices = std::move(GLTF::Import::get_Index(gltf, primitive));
            auto vertexes = std::move(GLTF::Import::get_Position3DNormalTangetBinomialUV(gltf, primitive, indices));
            // Swap Z Y
            if (m_mode & M_SWAP_ZY)
            {
                for (auto& vertex : vertexes)
                {
                    // Swap all
                    std::swap(vertex.m_position.z, vertex.m_position.y);
                    std::swap(vertex.m_normal.z, vertex.m_normal.y);
                    std::swap(vertex.m_tangent.z, vertex.m_tangent.y);
                    std::swap(vertex.m_binomial.z, vertex.m_binomial.y);
                }
            }
            // Force indexed
            if (indices.empty())
            {
                indices.reserve(vertexes.size());
                for (size_t i = 0; i < vertexes.size(); ++i)
                    indices.emplace_back(i);
            }
            // to LHs
            if (m_mode & M_TO_LHS)
            {
                // Flip
                for (auto& vertex : vertexes)
                {
                    // Flip Z for position and normal
                    vertex.m_position.z *= -1.0;
                    vertex.m_normal.z *= -1.0;
                    vertex.m_tangent.z *= -1.0;
                    // The bitangent is a direction as the tangent: mirrored the same way
                    vertex.m_binomial.z *= -1.0;
                }
                // Remap indices based on draw type
                switch (drawtype)
                {
                case Render::DrawType::DRAW_TRIANGLES:
                {
                    // Flip winding order for triangles (swap second and third indices)
                    for (size_t i = 0; i < indices.size(); i += 3)
                    {
                        if (i + 2 < indices.size())
                        {
                            std::swap(indices[i + 1], indices[i + 2]);
                        }
                    }
                    break;
                }
                case Render::DrawType::DRAW_TRIANGLE_STRIP:
                {
                    // For triangle strips, we need to flip every other triangle
                    // In triangle strips, each new vertex forms a triangle with the previous two
                    for (size_t i = 0; i < indices.size() - 2; i += 2)
                    {
                        std::swap(indices[i], indices[i + 1]);
                    }
                    break;
                }
                case Render::DrawType::DRAW_LINES:
                case Render::DrawType::DRAW_LINE_LOOP:
                case Render::DrawType::DRAW_POINTS:
                default:
                    // No need to swap indices for line loops, they don't have winding order
                    break;
                }
            }
            // Add sub mesh
            static_mesh_context.m_submesh.emplace_back(drawtype, indices.size(), context_index.size());
            // Update indices
            if (context_mesh.size())
            {
                for (auto& index : indices)
                    index += context_mesh.size();
            }
            // Its joints and their weights (none: the first joint, all of it)
            std::vector<Vec4> joints(vertexes.size(), Vec4(0.0f));
            std::vector<Vec4> weights(vertexes.size(), Vec4(1.0f, 0.0f, 0.0f, 0.0f));
            auto joints_it = primitive.attributes.find("JOINTS_0");
            auto weights_it = primitive.attributes.find("WEIGHTS_0");
            if (joints_it != primitive.attributes.end() && weights_it != primitive.attributes.end())
            {
                const std::vector<float> joint_values = GLTF::Import::read_floats(gltf, joints_it->second, false);
                const std::vector<float> weight_values = GLTF::Import::read_floats(gltf, weights_it->second, true);
                for (size_t i = 0; i < vertexes.size() && i * 4 + 3 < joint_values.size() && i * 4 + 3 < weight_values.size(); ++i)
                {
                    joints[i] = Vec4(joint_values[i * 4], joint_values[i * 4 + 1], joint_values[i * 4 + 2], joint_values[i * 4 + 3]);
                    Vec4 weight(weight_values[i * 4], weight_values[i * 4 + 1], weight_values[i * 4 + 2], weight_values[i * 4 + 3]);
                    const float sum = weight.x + weight.y + weight.z + weight.w;
                    weights[i] = sum > 0.0f ? weight / sum : Vec4(1.0f, 0.0f, 0.0f, 0.0f);
                }
                skinned = true;
            }
            // Push in context_mesh
            context_index.insert(context_index.end(), indices.begin(), indices.end());
            context_mesh.insert(context_mesh.end(), vertexes.begin(), vertexes.end());
            context_joints.insert(context_joints.end(), joints.begin(), joints.end());
            context_weights.insert(context_weights.end(), weights.begin(), weights.end());
        }
        break;
        default:
            break;
        }
        // Next primitive
        mesh_materials_ids.push_back(primitive.material);
        ++primitive_id;
    }
    // Compute obb just 1 time
    const unsigned char* points = reinterpret_cast<const unsigned char*>(context_mesh.data());
    const size_t vertex_size = sizeof(Render::Layout::Position3DNormalTangetBinomialUV);
    const size_t position_offset = offsetof(Render::Layout::Position3DNormalTangetBinomialUV, m_position);
    Geometry::OBoundingBox obb = Geometry::obounding_box_from_points(points, position_offset, vertex_size, context_mesh.size());
    // At worst its axis aligned box (a mesh the fit fails on: every mesh has a box)
    if (!obb.valid())
    {
        const Geometry::AABoundingBox aabb = Geometry::aabounding_from_points(points, position_offset, vertex_size, context_mesh.size());
        obb = Geometry::OBoundingBox(Mat3(1.0f), aabb.get_center(), aabb.get_extension());
        m_context.logger()->info("mesh " + mesh.name + ": its OBB from its AABB");
    }
    m_mesh_obbs.push_back(obb);
    // Serialize
    std::vector<unsigned char> buffer;
    size_t mesh_id = m_mesh_names.size();
    std::string sm3d_mesh_filename = m_names.make(mesh.name, "mesh" + std::to_string(mesh_id));
    std::string sm3d_mesh_path = Filesystem::join(m_output, sm3d_mesh_filename + ".sm3dgz");
    static_mesh_context.m_index = std::move(context_index);
    m_mesh_skin_points.emplace_back();
    if (skinned)
    {
        // The skinned layout: each vertex with its joints; its points, their strongest joint
        Render::Mesh::Vertex3DNTBUVSkinList skin_mesh;
        auto& points = m_mesh_skin_points.back();
        skin_mesh.reserve(context_mesh.size());
        points.reserve(context_mesh.size());
        for (size_t i = 0; i < context_mesh.size(); ++i)
        {
            const Vec4& joints = context_joints[i];
            const Vec4& weights = context_weights[i];
            skin_mesh.emplace_back(context_mesh[i], joints, weights);
            int strongest = 0;
            for (int c = 1; c < 4; ++c)
            {
                if (weights[c] > weights[strongest])
                {
                    strongest = c;
                }
            }
            points.push_back(Vec4(context_mesh[i].m_position, joints[strongest]));
        }
        static_mesh_context.m_vertex = std::move(skin_mesh);
    }
    else
    {
        static_mesh_context.m_vertex = std::move(context_mesh);
    }
    Parser::StaticMesh().serialize(static_mesh_context, buffer);
    Square::Filesystem::binary_compress_file_write_all(sm3d_mesh_path, buffer);
    m_mesh_name_files[sm3d_mesh_filename] = sm3d_mesh_path;
    m_mesh_names.emplace_back(std::move(sm3d_mesh_filename));
    return m_mesh_names.size();
}

bool MeshManager::skinned(size_t id) const
{
    return id < m_mesh_skin_points.size() && !m_mesh_skin_points[id].empty();
}

const std::vector<Square::Vec4>& MeshManager::skin_points(size_t id) const
{
    static const std::vector<Square::Vec4> none;
    return id < m_mesh_skin_points.size() ? m_mesh_skin_points[id] : none;
}
