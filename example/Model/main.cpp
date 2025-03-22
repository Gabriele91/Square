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

class ModelImporter : public Square::AppInterface
{
public:
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
        M_TO_LHS  = 0b00000010,
    };

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
        std::unordered_map<std::string, std::string> mesh_name_files;
        std::vector<std::string> mesh_names;
        std::vector<Square::Geometry::OBoundingBox> mesh_obbs;
        // Get meshes
        size_t mesh_id = 0;
        for (const auto& mesh : gltf_model.meshes)
        {
            //build context
            Parser::StaticMesh::Context static_mesh_context;
            // Mesh geometry 
            Render::Mesh::Vertex3DNTBUVList context_mesh;
            Render::Mesh::IndexList context_index;
            // Save
            size_t primitive_id = 0;
            for (const auto& primitive : mesh.primitives)
            {
                switch (GLTF::Import::determine_structure(primitive.attributes, gltf_model.accessors))
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
                        context().logger()->warning("Error to import primitive[" + std::to_string(primitive_id) + "] mesh: " + mesh.name);
                        continue;
                    }
                    // Get data
                    auto indices = std::move(GLTF::Import::get_Index(gltf_model, primitive));
                    auto vertexes = std::move(GLTF::Import::get_Position3DNormalTangetBinomialUV(gltf_model, primitive, indices));
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
                        for (auto& index : context_index)
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
                ++primitive_id;
            }
            // Compute obb just 1 time
            {
                const unsigned char* points = reinterpret_cast<const unsigned char*>( context_mesh.data() );
                const size_t vertex_size = sizeof(Render::Layout::Position3DNormalTangetBinomialUV);
                const size_t position_offset = offsetof(Render::Layout::Position3DNormalTangetBinomialUV, m_position);
                mesh_obbs.push_back(Geometry::obounding_box_from_points(points, position_offset, vertex_size, context_mesh.size()));
            }
            // Serialize
            std::vector<unsigned char> buffer;
            std::string sm3d_mesh_filename = m_output_model_name + "_mesh" + std::to_string(mesh_id);
            std::string sm3d_mesh_path = Filesystem::join(m_output_model_path, sm3d_mesh_filename + ".sm3dgz");
            static_mesh_context.m_index = std::move(context_index);
            static_mesh_context.m_vertex = std::move(context_mesh);
            Parser::StaticMesh().serialize(static_mesh_context, buffer);
            Square::Filesystem::binary_compress_file_write_all(sm3d_mesh_path, buffer);
            mesh_name_files[sm3d_mesh_filename] = sm3d_mesh_path;
            mesh_names.emplace_back(std::move(sm3d_mesh_filename));
            // Next mesh
            ++mesh_id;
        }
        // Create scene
        context().add_resource_map<Resource::Mesh>(mesh_name_files);
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
                            size_t mesh_id = node.content.value().m_id;
                            actor->component<StaticMesh>()->m_mesh = context().resource<Resource::Mesh>(mesh_names[mesh_id]);
                            actor->component<StaticMesh>()->m_material = context().resource<Resource::Material>("default");
                            actor->component<StaticMesh>()->set_obounding_box(mesh_obbs[mesh_id]);
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
    ModelImporter::OutputFormat output_model_format{ ModelImporter::OutputFormat::SQ_BIN_GZ };
    if (auto format_it = args.find("format"); format_it != args.end())
    if (auto format_str = std::get<std::string>(format_it->second); format_str.size())
    {
        if (Square::case_insensitive_equal(format_str, "bin"))
        {
            output_model_format = ModelImporter::OutputFormat::SQ_BIN;
        }
        else if (Square::case_insensitive_equal(format_str, "bgz"))
        {
            output_model_format = ModelImporter::OutputFormat::SQ_BIN_GZ;
        }
        else if (Square::case_insensitive_equal(format_str, "json"))
        {
            output_model_format = ModelImporter::OutputFormat::SQ_JSON;
        }
        else if (Square::case_insensitive_equal(format_str, "jgz"))
        {
            output_model_format = ModelImporter::OutputFormat::SQ_JSON_GZ;
        }
    }
    // Modes
    unsigned char modes = ModelImporter::M_NONE;
    // yzswap
    if (auto yzswap_it = args.find("swapzy"); yzswap_it != args.end())
    if (std::get<bool>(yzswap_it->second))
    {
        modes |= ModelImporter::M_SWAP_ZY;
    }
    // convert to lhs
    if (auto zforward_it = args.find("lhs"); zforward_it != args.end())
    if (std::get<bool>(zforward_it->second))
    {
        modes |= ModelImporter::M_TO_LHS;
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
