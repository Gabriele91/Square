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
#include <unordered_set>
#include <cctype>
#include <algorithm>
#include <filesystem>
#include "GLTFImport.h"
#include "SquareExtras.h"
#include "TextureManager.h"
#include "MaterialManager.h"
#include "MeshManager.h"

enum class OutputFormat
{
    SQ_BIN,
    SQ_BIN_GZ,
    SQ_JSON,
    SQ_JSON_GZ
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
    , Square::Shell::Command{ "images",  "m", "texture images [bc, astc, png, keep]"     , Square::Shell::ValueType::value_string, false, Square::Shell::Value_t(std::string("bc"))  }
    , Square::Shell::Command{ "pack",    "p", "pack the output folder in an archive (.sqz)", Square::Shell::ValueType::value_none  , false, Square::Shell::Value_t(false)              }
    , Square::Shell::Command{ "cascades","c", "cascades of the directional lights [1-8]"  , Square::Shell::ValueType::value_int   , false, Square::Shell::Value_t(0)                  }
    , Square::Shell::Command{ "help",    "h", "show help"                                , Square::Shell::ValueType::value_none  , false, Square::Shell::Value_t(false)              }
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
    bool m_convert_images;
    ImageConverter::Compression m_compression;
    bool m_pack{ false }; //--pack: the folder in <folder>.sqz (the engine reads it as the folder)
    int  m_cascades{ 0 }; //--cascades: of the directional lights (0: the default of the engine)

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
                  size_t shodow_resoluction = 0,
                  bool convert_images = true,
                  ImageConverter::Compression compression = ImageConverter::Compression::BC)
    : m_input_model_path(input_model_path)
    , m_output_model_path(output_model_path)
    , m_output_model_name(output_model_name)
    , m_output_model_format(output_model_format)
    , m_mode(mode)
    , m_shadow_resoluction(shodow_resoluction)
    , m_convert_images(convert_images)
    , m_compression(compression)
    {}

    virtual void start() 
    {
        using namespace Square;
        using namespace Square::Data;
        using namespace Square::Scene;
        // Add common resources
        context().add_resources(Filesystem::join(Filesystem::resource_dir(), "/resources.rs"));
        context().add_resources(Filesystem::join(Filesystem::resource_dir(), "/common/common.rs"));
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
        // The model folder: its files are named relative to it (the engine resolves them from there)
        const bool folder_created = !Filesystem::exists(m_output_model_path);
        if (folder_created && !Filesystem::makedir(m_output_model_path))
        {
            context().logger()->warning("Unable to create the output folder: " + m_output_model_path);
            return;
        }
        // Texture Manager
        TextureManager texture_manager(context(), m_output_model_path, gltf_model, m_convert_images, m_compression);
        MaterialManager material_manager(context(), m_output_model_path, texture_manager, gltf_model);
        MeshManager mesh_manager(context(), m_output_model_path, m_mode, gltf_model);
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
                    const Quat swap_zy = angle_axis(radians(90.0f), Constants::axis_x);
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
                                        std::string name = material_manager.at(mat_id).value_or("default");
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
                                    if (m_cascades)
                                        actor->component<DirectionLight>()->cascades(m_cascades);
                                }
                                break;
                                default: break;
                                }
                                // "square_<attribute>" (Blender custom properties) of the light
                                // data, then of the object: the attributes of the light
                                // (square_radius, square_shadow, square_visible...)
                                for (const auto& light : actor->components())
                                {
                                    SquareExtras::apply(*light.second, gltf_light.m_extras);
                                    SquareExtras::apply(*light.second, node.extras);
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
            std::ofstream ofile(actor_model_name, std::ios::out | std::ios::binary);
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
        // Pack: the folder in an archive, the folder removed (only when made here)
        if (m_pack) pack(folder_created);
    }

    void pack(bool remove_folder)
    {
        using namespace Square;
        std::string folder = m_output_model_path;
        while (folder.size() > 1 && (folder.back() == '/' || folder.back() == '\\')) folder.pop_back();
        const std::string archive = folder + ".sqz";
        if (!Filesystem::archive_write(folder, archive))
        {
            context().logger()->warning("Unable to pack the output folder in " + archive);
            return;
        }
        context().logger()->info("Packed in " + archive);
        std::error_code error;
        if (remove_folder) std::filesystem::remove_all(folder, error);
        else context().logger()->warning("The output folder was there before: kept, remove it (the archive has the same resources)");
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
    // images: bc (converted and compressed in DDS, the default), astc (compressed in KTX), png
    // (converted) or keep (copied as they are)
    bool convert_images = true;
    ImageConverter::Compression compression = ImageConverter::Compression::BC;
    if (auto images_it = args.find("images"); images_it != args.end())
    if (auto images_str = std::get<std::string>(images_it->second); images_str.size())
    {
        if (Square::case_insensitive_equal(images_str, "keep"))
        {
            convert_images = false;
            compression = ImageConverter::Compression::NONE;
        }
        else if (Square::case_insensitive_equal(images_str, "png"))
        {
            compression = ImageConverter::Compression::NONE;
        }
        else if (Square::case_insensitive_equal(images_str, "astc"))
        {
            compression = ImageConverter::Compression::ASTC;
        }
        else if (!Square::case_insensitive_equal(images_str, "bc"))
        {
            std::cout << "unknown images mode: " << images_str << " (bc, astc, png, keep)" << std::endl;
            return -1;
        }
    }
    //the importer, packed in an archive (--pack) or not
    auto* importer = new ModelImporter(input_model_path, output_model_path, output_model_name, output_model_format, modes, shadow_resoluction, convert_images, compression);
    if (auto pack_it = args.find("pack"); pack_it != args.end()) importer->m_pack = std::get<bool>(pack_it->second);
    if (auto cascades_it = args.find("cascades"); cascades_it != args.end()) importer->m_cascades = std::get<int>(cascades_it->second);
    //srgb on
    const bool srgb = true;
    //a tool: no splash screen
    app.splash_screen(false);
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
        , srgb                                 // SRGB
        , false                                // Debug
      }
    , "ModelImporter"
    , importer
    );
    // End
    return 0;
}
