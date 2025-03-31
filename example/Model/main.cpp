//
//  Square
//
//  Created by Gabriele Di Bari on 18/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#define SQUARE_MAIN
#include <Square/Square.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include "GLTFImport.h"

enum class OutputFormat
{
    SQ_BIN,
    SQ_BIN_GZ,
    SQ_JSON,
    SQ_JSON_GZ
};

enum Modes : unsigned char
{
    M_NONE = 0,
    M_SWAP_ZY = 0b00000001,
    M_TO_LHS = 0b00000010,
};

static Square::Shell::ParserCommands s_ShellCommands
{
      Square::Shell::Command{ "input",   "i", "input model [gltf, glb]"                  , Square::Shell::ValueType::value_string, true,  Square::Shell::Value_t()                   }
    , Square::Shell::Command{ "output",  "o", "output model path"                        , Square::Shell::ValueType::value_string, true,  Square::Shell::Value_t()                   }
    , Square::Shell::Command{ "format",  "f", "output model format [bin, bgz, json, jgz]", Square::Shell::ValueType::value_string, false, Square::Shell::Value_t(std::string("bgz")) }
    , Square::Shell::Command{ "name",    "n", "output model name"                        , Square::Shell::ValueType::value_string, false, Square::Shell::Value_t()                   }
    , Square::Shell::Command{ "debug",   "d", "enable debug"                             , Square::Shell::ValueType::value_none  , false, Square::Shell::Value_t(false)              }
    , Square::Shell::Command{ "swapzy",  "s", "swap z with y coord"                      , Square::Shell::ValueType::value_none  , false, Square::Shell::Value_t(false)              }
    , Square::Shell::Command{ "lhs",     "l", "convert in left hand"                     , Square::Shell::ValueType::value_none  , false, Square::Shell::Value_t(true)               }
    , Square::Shell::Command{ "shadow",  "r", "force shadow resolution [size]"           , Square::Shell::ValueType::value_int   , false, Square::Shell::Value_t(0)                  }
    , Square::Shell::Command{ "help",    "h", "show help"                                , Square::Shell::ValueType::value_none  , false, Square::Shell::Value_t(false)              }
};

class TextureManager
{
    struct TextureBufferDescription
    {
        std::string m_name;
        size_t m_index;
    };
    using TextureType = std::variant< std::string, TextureBufferDescription >;

    Square::Context& m_context;
    std::string m_output;
    std::vector< TextureType > m_images;
    std::vector< std::string > m_samplers;
    std::vector< std::string > m_textures;

    inline static const char* texture_filter_to_string(const Square::Data::GLTF::TextureFilter filter)
    {
        switch (filter) 
        {
            case Square::Data::GLTF::TextureFilter::NEAREST: return "nearest";
            case Square::Data::GLTF::TextureFilter::LINEAR: return "linear";
            case Square::Data::GLTF::TextureFilter::NEAREST_MIPMAP_NEAREST: return "nearest_mipmap_nearest";
            case Square::Data::GLTF::TextureFilter::LINEAR_MIPMAP_NEAREST: return "linear_mipmap_nearest";
            case Square::Data::GLTF::TextureFilter::NEAREST_MIPMAP_LINEAR: return "nearest_mipmap_linear";
            case Square::Data::GLTF::TextureFilter::LINEAR_MIPMAP_LINEAR: return "linear_mipmap_linear";
            default: return "unknown";
        }
    }

    inline static const char* texture_wrap_mode_to_string(const Square::Data::GLTF::TextureWrapMode mode)
    {
        switch (mode) 
        {
        case Square::Data::GLTF::TextureWrapMode::CLAMP_TO_EDGE: return "clamp";
        case Square::Data::GLTF::TextureWrapMode::MIRRORED_REPEAT: return "mirrored_repeat";
        case Square::Data::GLTF::TextureWrapMode::REPEAT: return "repeat";
        default: return "unknown";
        }
    }

public:
    TextureManager(Square::Context& context, const std::string& output) 
    : m_context(context)
    , m_output(output) 
    {}
    TextureManager(Square::Context& context, const std::string& output, const Square::Data::GLTF::GLTF& gltf)
    : m_context(context)
    , m_output(output) 
    {
        for (auto& image : gltf.images)
        {
            add_image(image);
        }
        for (auto& sempler : gltf.samplers)
        {
            add_sampler(sempler);
        }
        for (auto& texture : gltf.textures)
        {
            add_texture(texture, gltf.views, gltf.buffers);
        }
    }

    size_t add_image(const Square::Data::GLTF::Image& in_image)
    {
        if (std::holds_alternative<Square::Data::GLTF::ImagePath>(in_image))
        {
            const Square::Data::GLTF::ImagePath& image_path = std::get<Square::Data::GLTF::ImagePath>(in_image);
            size_t index = m_images.size();
            // Build path
            auto newname = Square::Filesystem::get_basename(image_path.uri)
                         + "_" + std::to_string(index)
                         + Square::Filesystem::get_extension(image_path.uri);
            auto outputpath = Square::Filesystem::join(m_output, newname);
            // Copy
            if (!Square::Filesystem::copyfile(image_path.uri, outputpath))
            {
                m_context.logger()->warning("unable to copy: " + image_path.uri);
            }
            m_images.push_back(outputpath);
            return m_images.size();
        }
        else if (std::holds_alternative<Square::Data::GLTF::ImageBuffer>(in_image))
        {
            const Square::Data::GLTF::ImageBuffer& buffer_description = std::get<Square::Data::GLTF::ImageBuffer>(in_image);
            m_images.push_back(TextureBufferDescription{ buffer_description.name, buffer_description.buffer_view });
            return m_images.size();
        }
        
        m_context.logger()->warning("Invalid image");
        return ~size_t(0);
    }
    size_t add_sampler(const Square::Data::GLTF::Sampler& sampler)
    {
        const char _template[] =
        {
            "mag_filter %s\n"
            "min_filter %s\n"
            "wrap_s %s\n"
            "wrap_t %s\n"
            "wrap_r %s\n"
        };
        char output_template[255] = { '\0' };
        std::snprintf(&output_template[0], 255, _template,
            texture_filter_to_string(sampler.mag_filter),
            texture_filter_to_string(sampler.min_filter),
            texture_wrap_mode_to_string(sampler.wrap_s),
            texture_wrap_mode_to_string(sampler.wrap_t),
            texture_wrap_mode_to_string(sampler.wrap_r)
        );
        m_samplers.push_back(output_template);
        return m_samplers.size();
    }
    size_t add_texture(const Square::Data::GLTF::Texture& texture, const Square::Data::GLTF::Views& views, const Square::Data::GLTF::Buffers& buffers)
    {
        if (texture.sampler < m_samplers.size() && texture.source < m_images.size())
        {
            const auto& in_image = m_images[texture.source];
            const auto& sampler = m_samplers[texture.sampler];
            std::string texture_id = "_" + std::to_string(m_textures.size());
            if (std::holds_alternative<std::string>(in_image))
            {
                std::string image_uri = std::get<std::string>(in_image);
                std::string texture_sampler_body = sampler + "url " + Square::Filesystem::get_filename(image_uri) + "\n";
                std::string texture_sampler_path = Square::Filesystem::join(m_output, Square::Filesystem::get_basename(image_uri) + texture_id + ".sqtex");

                Square::Filesystem::text_file_write_all(texture_sampler_path, texture_sampler_body);
                m_textures.push_back(texture_sampler_path);
                return m_textures.size();
            }
            else if (std::holds_alternative<TextureBufferDescription>(in_image))
            {
                const TextureBufferDescription& image_buffer_description = std::get<TextureBufferDescription>(in_image);
                std::string texture_path = Square::Filesystem::join(m_output, image_buffer_description.m_name + texture_id + ".sqtex");
                std::string texture_body = sampler + "data";
                // Get buffer
                if (image_buffer_description.m_index < views.size())
                {
                    const auto& image_data_view = views[image_buffer_description.m_index];
                    const auto& image_buffer = buffers[image_data_view.buffer];
                    /////////////////////////////////////////////////////////////////////
                    FILE* texture_pfile = std::fopen(texture_path.c_str(), "wb");
                    //bad case
                    if (texture_pfile)
                    {
                        std::fwrite(texture_body.data(), texture_body.size(), 1, texture_pfile);
                        std::fwrite(&image_buffer[image_data_view.offset], image_data_view.length, 1, texture_pfile);
                        /////////////////////////////////////////////////////////////////////
                        std::fclose(texture_pfile);
                        /////////////////////////////////////////////////////////////////////
                        m_textures.push_back(texture_path);
                        return m_textures.size();
                    }
                }
            }
        }
        m_context.logger()->warning("Invalid texture");
        return ~size_t(0);
    }

    std::optional<std::string> at(size_t index) const
    {
        std::optional<std::string> texture { };
        if (index < m_textures.size())
        {
            return Square::Filesystem::get_basename(m_textures[index]);
        }
        return texture;
    }
};

class MaterialManager
{
    std::unordered_map<std::string, std::string> m_materials_resrouces;
    std::vector< std::string > m_materials;
    Square::Context& m_context;
    std::string m_output;

    static inline std::string template_material(const Square::Data::GLTF::Material& material, const TextureManager& texture_manager)
    {
        const char solit_material_template[] =
        {
            "effect \"Solid\"\n"
            "{\n"
                "\tmask float(%f)\n"
                "\tcolor Vec4(%f,%f,%f,%f)\n"
                "\tdiffuse_map  texture(\"%s\")\n"
                "\tnormal_map   texture(\"%s\")\n"
                "\tspecular_map texture(\"%s\")\n"
                "\tocclusion_map texture(\"%s\")\n"
            "}"
        };
        float tmask = material.alpha_cutoff;
        Square::Vec4 tcolor ( material.emissive_factor, 1.0f );
        std::string diffuse_map = material.pbr_metallic_roughness.has_value() && material.pbr_metallic_roughness.value().base_color_texture.has_value()
                                ? texture_manager.at(material.pbr_metallic_roughness.value().base_color_texture.value().index).value_or("default") 
                                : "default";
        std::string normal_map = material.normal_texture.has_value() ?
                                  texture_manager.at(material.normal_texture.value().index).value_or("normal_up") :
                                  "normal_up";
        std::string specular_map = material.emissive_texture.has_value() ?
                                  texture_manager.at(material.emissive_texture.value().index).value_or("black") :
                                  "black";
        std::string occlusion_map = material.occlusion_texture.has_value() ?
                                  texture_manager.at(material.occlusion_texture.value().index).value_or("white") :
                                  "white";

        char output_template[255] = { '\0' };
        std::snprintf(&output_template[0], 255, solit_material_template, 
            tmask,
            tcolor.x, tcolor.y, tcolor.z, tcolor.w,
            diffuse_map.c_str(),
            normal_map.c_str(),
            specular_map.c_str(),
            occlusion_map.c_str()
        );
        return output_template;
    }

public:
    MaterialManager(Square::Context& context, const std::string& output)
    : m_context(context)
    , m_output(output) 
    {}
    
    MaterialManager(Square::Context& context, const std::string& output, const TextureManager& texture_manager, const Square::Data::GLTF::GLTF& gltf)
    : m_context(context)
    , m_output(output) 
    {
        for (auto& material : gltf.materials)
        {
            add_material(material, texture_manager);
        }
    }

    const std::unordered_map<std::string, std::string>& resource_map() const
    {
        return m_materials_resrouces;
    }

    std::optional<std::string> at(size_t id)
    {
        return id < m_materials.size()
               ? std::optional<std::string> { m_materials[id] } 
               : std::optional<std::string>{};
    }

    size_t add_material(const Square::Data::GLTF::Material& material, const TextureManager& texture_manager)
    {
        const std::string material_data = template_material(material, texture_manager);
        const std::string material_name = material.name + "_" + std::to_string(m_materials.size());
        const std::string material_path = Square::Filesystem::join(m_output, material_name + ".mat");
        Square::Filesystem::text_file_write_all(material_path, material_data);
        m_materials.push_back(material_name);
        m_materials_resrouces[material_name] = material_path;
        return m_materials.size();
    }
};

class MeshManager
{
    Square::Context& m_context;
    std::string m_mesh_prefix;
    std::string m_output;
    unsigned char m_mode{ M_NONE };

    std::unordered_map<std::string, std::string> m_mesh_name_files;
    std::vector< std::vector<size_t> > m_meshes_materials;
    std::vector<std::string> m_mesh_names;
    std::vector<Square::Geometry::OBoundingBox> m_mesh_obbs;

public:
    MeshManager(Square::Context& context, const std::string& mesh_prefix, const std::string& output, unsigned char mode)
    : m_context(context)
    , m_mesh_prefix(mesh_prefix)
    , m_output(output)
    , m_mode(mode)
    {}

    MeshManager(Square::Context& context, const std::string& mesh_prefix, const std::string& output, unsigned char mode, const Square::Data::GLTF::GLTF& gltf)
    : m_context(context)
    , m_mesh_prefix(mesh_prefix)
    , m_output(output)
    , m_mode(mode)
    {
        for (auto& mesh : gltf.meshes)
        {
            add_mesh(mesh, gltf);
        }
    }

    const std::unordered_map<std::string, std::string>& resource_map() const
    {
        return m_mesh_name_files;
    }

    std::optional< std::tuple<const std::string*, const Square::Geometry::OBoundingBox*, const std::vector<size_t>* > > at(size_t id) const
    {
        if (id < m_mesh_names.size() && id < m_mesh_obbs.size())
        {
            return { std::make_tuple(&m_mesh_names[id], &m_mesh_obbs[id], &m_meshes_materials[id]) };
        }
        return {};
    }

    size_t add_mesh(const Square::Data::GLTF::Mesh& mesh, const Square::Data::GLTF::GLTF& gltf)
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
                        // Flip bitangent to maintain correct handedness
                        vertex.m_binomial = -vertex.m_binomial;
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
                // Push in context_mesh
                context_index.insert(context_index.end(), indices.begin(), indices.end());
                context_mesh.insert(context_mesh.end(), vertexes.begin(), vertexes.end());
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
        m_mesh_obbs.push_back(Geometry::obounding_box_from_points(points, position_offset, vertex_size, context_mesh.size()));
        // Serialize
        std::vector<unsigned char> buffer;
        size_t mesh_id = m_mesh_names.size();
        std::string sm3d_mesh_filename = m_mesh_prefix + "_mesh" + std::to_string(mesh_id);
        std::string sm3d_mesh_path = Filesystem::join(m_output, sm3d_mesh_filename + ".sm3dgz");
        static_mesh_context.m_index = std::move(context_index);
        static_mesh_context.m_vertex = std::move(context_mesh);
        Parser::StaticMesh().serialize(static_mesh_context, buffer);
        Square::Filesystem::binary_compress_file_write_all(sm3d_mesh_path, buffer);
        m_mesh_name_files[sm3d_mesh_filename] = sm3d_mesh_path;
        m_mesh_names.emplace_back(std::move(sm3d_mesh_filename));
        return m_mesh_names.size();
    }
};

class ModelImporter : public Square::AppInterface
{
public:
    unsigned char m_mode{ M_NONE };
    std::string m_input_model_path;
    std::string m_output_model_path;
    std::string m_output_model_name;
    OutputFormat m_output_model_format;
    size_t m_shadow_resoluction;

    struct Consts
    {
        // Candela to Power, blender: 
        // LUMENS_PER_WATT = 683 (standard value for monochromatic 555nm light)
        static constexpr double LUMENS_PER_WATT = 683;
        // Inner light factors
        static constexpr double SOFT_SPOTLIGHT_FACTOR = 0.1;
        static constexpr double SOFT_POINTLIGHT_FACTOR = 0.5;
    };

    static const double blender_candela_to_power(const double intensity)
    {
        return (intensity * Square::Constants::pi<double>() * 4) / Consts::LUMENS_PER_WATT;
    }

    ModelImporter(const std::string& input_model_path, 
                  const std::string& output_model_path,
                  const std::string& output_model_name, 
                  OutputFormat output_model_format,
                  unsigned char mode = M_NONE,
                  size_t shodow_resoluction = 0)
    : m_input_model_path(input_model_path)
    , m_output_model_path(output_model_path)
    , m_output_model_name(output_model_name)
    , m_output_model_format(output_model_format)
    , m_mode(mode)
    , m_shadow_resoluction(shodow_resoluction)
    {}

    virtual void start() 
    {
        using namespace Square;
        using namespace Square::Data;
        using namespace Square::Scene;
        // Add common resources
        context().add_resources(Filesystem::join(Filesystem::resource_dir(), "/resources.rs"));
        // Load
        auto loaded_model = GLTF::load(m_input_model_path);
        // Test
        if (std::holds_alternative<std::string>(loaded_model))
        {
            context().logger()->warning("Error to parse model: " + std::get<std::string>(loaded_model));
            return;
        }
        // Get model
        const auto& gltf_model = std::get<GLTF::GLTF>(loaded_model);
        // Texture Manager
        TextureManager texture_manager(context(), m_output_model_path, gltf_model);
        MaterialManager material_manager(context(), m_output_model_path, texture_manager, gltf_model);
        MeshManager mesh_manager(context(), m_output_model_name, m_output_model_path, m_mode, gltf_model);
        // Create scene
        context().add_resource_map<Resource::Mesh>(mesh_manager.resource_map());
        context().add_resource_map<Resource::Material>(material_manager.resource_map());
        Shared<Actor> main_node = Square::MakeShared<Actor>(context());
        main_node->name(m_output_model_name);
        GLTF::Import::visit_default_scene< Shared<Actor> >(gltf_model, main_node,
            [&](const GLTF::Node* const parent, const GLTF::Node& node, Shared<Actor>& parent_actor) -> Shared<Actor>
            {
                Shared<Actor> actor = MakeShared<Actor>(context());
                actor->name(node.name);
                Vec3 translation{ 0.0f,0.0f,0.0f }, scale{ 1.0f,1.0f,1.0f };
                Quat rotation(0.0f,0.0f,0.0f,1.0f);
                if (node.transform_type & GLTF::TransformType::MATRIX)
                {
                    decompose_mat4(node.matrix, translation, rotation, scale);
                }
                else
                {
                    if (node.transform_type & GLTF::TransformType::TRANSLATION)
                    {
                        translation = node.translation;
                    }
                    if (node.transform_type & GLTF::TransformType::ROTATION)
                    {
                        rotation = node.rotation;
                    }
                    if (node.transform_type & GLTF::TransformType::SCALE)
                    {
                        scale = node.scale;
                    }
                }
                if (m_mode & M_SWAP_ZY)
                {
                    // Swap Y Z
                    const Quat swap_zy = angle_axis(radians(90.0f), Vec3(1, 0, 0));
                    // Swap all
                    std::swap(translation.z, translation.y);
                    rotation = swap_zy * rotation * conjugate(swap_zy);
                    std::swap(scale.z, scale.y);
                }
                if (m_mode & M_TO_LHS)
                {
                    translation.z = translation.z != 0.0f ? -translation.z : translation.z;
                    rotation = Quat(rotation.x, rotation.y, -rotation.z, -rotation.w);
                }
                actor->position(translation);
                actor->rotation(rotation);
                actor->scale(scale);
                // Set static mesh
                if (node.content.has_value())
                {
                    switch (node.content.value().m_type)
                    {
                        case GLTF::Node::NodeContentType::NT_MESH:
                        {
                            auto mesh_info = mesh_manager.at(node.content.value().m_id);
                            if (mesh_info.has_value())
                            {
                                // Unpack
                                auto& [mesh_name, mesh_obb, mesh_mats] = *mesh_info;
                                // Set
                                actor->component<StaticMesh>()->m_mesh = context().resource<Resource::Mesh>(*mesh_name);
                                if (auto mesh = actor->component<StaticMesh>()->m_mesh)
                                {
                                    size_t i = 0;
                                    for (; i < mesh->number_of_sub_meshs() && i < mesh_mats->size(); ++i)
                                    {
                                        // Get material id
                                        size_t mat_id = mesh_mats->at(i);
                                        std::string name = material_manager.at(i).value_or("default");
                                        actor->component<StaticMesh>()->m_materials.push_back(context().resource<Resource::Material>(name));
                                    }
                                    for (; i < mesh->number_of_sub_meshs(); ++i)
                                    {
                                        actor->component<StaticMesh>()->m_materials.push_back(context().resource<Resource::Material>("default"));
                                    }
                                }
                                actor->component<StaticMesh>()->set_obounding_box(*mesh_obb);

                            }
                        }
                        break;
                        case GLTF::Node::NodeContentType::NT_CAMERA:
                        {
                            size_t camera_id = node.content.value().m_id;
                            if (camera_id < gltf_model.cameras.size())
                            {
                                auto& gltf_camera = gltf_model.cameras[camera_id];
                                if (std::holds_alternative<GLTF::CameraProspective>(gltf_camera))
                                {
                                    auto& gltf_camera_pro = std::get<GLTF::CameraProspective>(gltf_camera);
                                    actor->component<Camera>()->perspective(
                                        gltf_camera_pro.m_yfov,
                                        gltf_camera_pro.m_aspect_ratio,
                                        gltf_camera_pro.m_znear,
                                        gltf_camera_pro.m_zfar
                                    );
                                }
                                else if (std::holds_alternative<GLTF::CameraOrthographic>(gltf_camera))
                                {
                                    auto& gltf_camera_ortho = std::get<GLTF::CameraOrthographic>(gltf_camera);
                                    actor->component<Camera>()->ortogonal(
                                        0, gltf_camera_ortho.m_xmag, 
                                        0, gltf_camera_ortho.m_ymag, 
                                        gltf_camera_ortho.m_znear, 
                                        gltf_camera_ortho.m_zfar
                                    );
                                }
                                else
                                {
                                    actor->component<Camera>();
                                }
                            }
                        }
                        break;
                        case GLTF::Node::NodeContentType::NT_LIGHT:
                        {
                            size_t light_id = node.content.value().m_id;
                            if (light_id < gltf_model.lights.size())
                            {
                                auto& gltf_light = gltf_model.lights[light_id];
                                switch (gltf_light.m_type)
                                {
                                case GLTF::Light::LightType::LT_SPOTLIGHT:
                                {
                                    actor->component<SpotLight>()->diffuse(gltf_light.m_color);
                                    actor->component<SpotLight>()->specular(gltf_light.m_color);
                                    actor->component<SpotLight>()->constant(1.0);
                                    if (m_shadow_resoluction)
                                        actor->component<SpotLight>()->shadow({ m_shadow_resoluction,m_shadow_resoluction });
                                    // Range
                                    if (gltf_light.m_range.has_value())
                                    {
                                        actor->component<SpotLight>()->radius(gltf_light.m_range.value());
                                        actor->component<SpotLight>()->inside_radius(gltf_light.m_range.value() * Consts::SOFT_SPOTLIGHT_FACTOR);
                                    }
                                    else
                                    {
                                        double power = blender_candela_to_power(gltf_light.m_intensity);
                                        actor->component<SpotLight>()->radius(power);
                                        actor->component<SpotLight>()->inside_radius(power * Consts::SOFT_SPOTLIGHT_FACTOR);
                                    }
                                    // Cut off
                                    if (gltf_light.m_spotfields.has_value())
                                    {
                                        actor->component<SpotLight>()->inner_cut_off(gltf_light.m_spotfields->m_inner_cone_angle);
                                        actor->component<SpotLight>()->outer_cut_off(gltf_light.m_spotfields->m_outer_cone_angle);
                                    }
                                }
                                break;
                                case GLTF::Light::LightType::LT_POINT:
                                {
                                    actor->component<PointLight>()->diffuse(gltf_light.m_color);
                                    actor->component<PointLight>()->specular(gltf_light.m_color);
                                    actor->component<PointLight>()->constant(1.0);
                                    if (m_shadow_resoluction)
                                        actor->component<PointLight>()->shadow({ m_shadow_resoluction,m_shadow_resoluction });
                                    if (gltf_light.m_range.has_value())
                                    {
                                        actor->component<PointLight>()->radius(gltf_light.m_range.value());
                                        actor->component<PointLight>()->inside_radius(gltf_light.m_range.value() * Consts::SOFT_POINTLIGHT_FACTOR);
                                    }
                                    else
                                    {
                                        double power = blender_candela_to_power(gltf_light.m_intensity);
                                        actor->component<PointLight>()->radius(power);
                                        actor->component<PointLight>()->inside_radius(power * 0.1);
                                    }
                                }
                                break;
                                case GLTF::Light::LightType::LT_DIRECTIONAL:
                                {
                                    actor->component<DirectionLight>()->diffuse(gltf_light.m_color);
                                    actor->component<DirectionLight>()->specular(gltf_light.m_color);
                                    if (m_shadow_resoluction)
                                        actor->component<DirectionLight>()->shadow({ m_shadow_resoluction,m_shadow_resoluction });
                                }
                                break;
                                default: break;
                                }
                            }
                        }
                        break;
                    default:
                    break;
                    }

                }
                parent_actor->add(actor);
                return actor;
            });
        // Serialize
        switch (m_output_model_format)
        {
        default:
        case OutputFormat::SQ_BIN:
        {
            using namespace Square::Data;
            using namespace Square::Filesystem::Stream;
            std::string actor_model_name = Filesystem::join(m_output_model_path, m_output_model_name + ".ac");
            std::ofstream ofile(actor_model_name);
            ArchiveBinWrite out(context(), ofile);
            main_node->serialize(out);
        }
        break;
        case OutputFormat::SQ_BIN_GZ:
        {
            using namespace Square::Data;
            using namespace Square::Filesystem::Stream;
            std::string actor_model_name = Filesystem::join(m_output_model_path, m_output_model_name + ".acgz");
            GZOStream ofile(actor_model_name);
            ArchiveBinWrite out(context(), ofile);
            main_node->serialize(out);
        }
        break;
        case OutputFormat::SQ_JSON:
        {
            using namespace Square;
            using namespace Square::Data;
            Json jout = Json(JsonObject());
            main_node->serialize_json(jout);
            std::string actor_model_name = Filesystem::join(m_output_model_path, m_output_model_name + ".acj");
            std::ofstream(actor_model_name) << jout;
        }
        break;
        case OutputFormat::SQ_JSON_GZ:
        {
            using namespace Square;
            using namespace Square::Data;
            using namespace Square::Filesystem::Stream;
            Json jout = Json(JsonObject());
            main_node->serialize_json(jout);
            std::string actor_model_name = Filesystem::join(m_output_model_path, m_output_model_name + ".acjgz");
            GZOStream(actor_model_name) << jout;
        }
        break;
        }
    }
    virtual bool run(double delta_time) { return false; };
    virtual bool end() { return true; };

};


square_main(s_ShellCommands)(Square::Application& app, Square::Shell::ParserValue& args, Square::Shell::Error& errors)
{
    using namespace Square;
    using namespace Square::Data;
    using namespace Square::Scene;
	// Show help:
	if (args.find("help") != args.end() && std::get<bool>(args["help"]))
	{
        app.context()->logger()->info((std::get<std::string>(args[Shell::__filename__]) + ":\n"));
        app.context()->logger()->info(Shell::help(s_ShellCommands));
        return 0;
	}
	// Test error
	if (errors.type != Shell::ErrorType::none)
	{
        app.context()->logger()->error("Error to parse input [" + std::to_string(errors.id_argument) + "]: " + errors.what);
        return -1;
	}
    // Get mandatory params
    const std::string& input_model_path = std::get<std::string>(args.at("input"));
    const std::string& output_model_path = std::get<std::string>(args.at("output"));
    //  Get name
    std::string  output_model_name = Filesystem::get_basename(output_model_path);
    if (auto name_it = args.find("name"); name_it != args.end())
    if (auto name = std::get<std::string>(name_it->second); name.size())
    {
        output_model_name = name;
    }
    // Get format
    OutputFormat output_model_format{ OutputFormat::SQ_BIN_GZ };
    if (auto format_it = args.find("format"); format_it != args.end())
    if (auto format_str = std::get<std::string>(format_it->second); format_str.size())
    {
        if (Square::case_insensitive_equal(format_str, "bin"))
        {
            output_model_format = OutputFormat::SQ_BIN;
        }
        else if (Square::case_insensitive_equal(format_str, "bgz"))
        {
            output_model_format = OutputFormat::SQ_BIN_GZ;
        }
        else if (Square::case_insensitive_equal(format_str, "json"))
        {
            output_model_format = OutputFormat::SQ_JSON;
        }
        else if (Square::case_insensitive_equal(format_str, "jgz"))
        {
            output_model_format = OutputFormat::SQ_JSON_GZ;
        }
    }
    // Modes
    unsigned char modes = Modes::M_NONE;
    // yzswap
    if (auto yzswap_it = args.find("swapzy"); yzswap_it != args.end())
    if (std::get<bool>(yzswap_it->second))
    {
        modes |= Modes::M_SWAP_ZY;
    }
    // convert to lhs
    if (auto zforward_it = args.find("lhs"); zforward_it != args.end())
    if (std::get<bool>(zforward_it->second))
    {
        modes |= Modes::M_TO_LHS;
    }
    // get shadow res
    int shadow_resoluction = 0;
    if (auto shadow_it = args.find("shadow"); shadow_it != args.end())
    {
        shadow_resoluction = std::get<int>(shadow_it->second);
    }
    //test
    app.execute
	(
      WindowSizePixel({ 640, 480 })
    , WindowMode::NOT_RESIZABLE
	, WindowRenderDriver
      { 
         Render::RenderDriver::DR_OPENGL, 4, 1 // DRIVER and Version 
        , 24, 8                                // Colors and depth
        , GpuType::GPU_HIGH                    // GPU type
        , false                                // Debug
      }
    , "ModelImporter"
    , new ModelImporter(input_model_path, output_model_path, output_model_name, output_model_format, modes, shadow_resoluction)
    );
    // End
    return 0;
}
