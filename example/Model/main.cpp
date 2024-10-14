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
      Square::Shell::Command{ "input",  "i", "input model [gltf, glb]"                  , Square::Shell::ValueType::value_string, true,  Square::Shell::Value_t()                   }
    , Square::Shell::Command{ "output", "o", "output model path"                        , Square::Shell::ValueType::value_string, true,  Square::Shell::Value_t()                   }
    , Square::Shell::Command{ "format", "f", "output model format [bin, bgz, json, jgz]", Square::Shell::ValueType::value_string, false, Square::Shell::Value_t(std::string("bgz")) }
    , Square::Shell::Command{ "name",   "n", "output model name"                        , Square::Shell::ValueType::value_string, false, Square::Shell::Value_t()                   }
    , Square::Shell::Command{ "debug",  "d", "enable debug"                             , Square::Shell::ValueType::value_none  , false, Square::Shell::Value_t(false)              }
    , Square::Shell::Command{ "help",   "h", "show help"                                , Square::Shell::ValueType::value_none  , false, Square::Shell::Value_t(false)              }
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

    std::string m_input_model_path;
    std::string m_output_model_path;
    std::string m_output_model_name;
    OutputFormat m_output_model_format;

    ModelImporter(const std::string& input_model_path, const std::string& output_model_path, const std::string& output_model_name, OutputFormat output_model_format)
    : m_input_model_path(input_model_path)
    , m_output_model_path(output_model_path)
    , m_output_model_name(output_model_name)
    , m_output_model_format(output_model_format)
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
                    // Force indexed
                    if (indices.empty())
                    {
                        indices.reserve(vertexes.size());
                        for (size_t i = 0; i < vertexes.size(); ++i)
                            indices.emplace_back(i);
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
                if (node.transform_type & GLTF::TransformType::TRANSLATION)
                {
                    actor->translation(node.translation);
                }
                if (node.transform_type & GLTF::TransformType::ROTATION)
                {
                    actor->rotation(node.rotation);
                }
                if (node.transform_type & GLTF::TransformType::SCALE)
                {
                    actor->scale(node.scale);
                }
                if (node.transform_type & GLTF::TransformType::MATRIX)
                {
                    Vec3 translation, scale;
                    Quat rotation;
                    decompose_mat4(node.matrix, translation, rotation, scale);
                    actor->position(translation);
                    actor->rotation(rotation);
                    actor->scale(scale);
                }
                // Set static mesh
                if (node.mesh.has_value())
                {
                    actor->component<StaticMesh>()->m_mesh = context().resource<Resource::Mesh>(mesh_names[*node.mesh]);
                    actor->component<StaticMesh>()->m_material = context().resource<Resource::Material>("default");
                    if(!actor->component<StaticMesh>()->build_local_obounding_box())
                        context().logger()->warning("Faild to compute the OBB: " + mesh_names[*node.mesh]);

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
    //test
    app.execute
	(
      WindowSizePixel({ 640, 480 })
    , WindowMode::NOT_RESIZABLE
	, WindowRenderDriver
      { 
         Render::RenderDriver::DR_OPENGL, 4, 1 // DRIVER and Version 
        , 24, 8                                // Colors and depth
        , GpuType::GPU_HIGH                   // GPU type
        , false                                // Debug
      }
    , "ModelImporter"
    , new ModelImporter(input_model_path, output_model_path, output_model_name, output_model_format)
    );
    // End
    return 0;
}
