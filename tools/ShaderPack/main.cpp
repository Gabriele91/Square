//
//  ShaderPack
//  Square
//
//  The offline side of the intermediate cache of the shaders (Resource/ShaderCache.h): the pack
//  distributed with a game, its shaders translated for every target of a platform (the cross
//  product of its languages, of the capabilities of their drivers, of the gamma corrections:
//  Resource::ShaderTarget::of_platform), so the runtime finds the ones of its driver whatever
//  machine of the platform it runs on, and translates nothing:
//  - windows: GLSL, HLSL, SPIR-V (Vulkan);
//  - macos: GLSL, MSL;
//  - ios: GLSL ES, MSL (iOS);
//  - linux: GLSL, GLSL ES, SPIR-V;
//  - android: GLSL ES, SPIR-V.
//  Every effect and shader of some resource files loaded once per target (each effect: all its
//  techniques, all their variants), translated only (no shader of the driver: the backend started
//  is any), their translations written in a folder (<folder>/<language>/<hash>.sqsh: the ones of
//  the languages of the platform made again), then the folder into an archive (shaders.sqz), read
//  by the engine before its own cache (next to the resources: Filesystem::resource_dir).
//
//      ShaderPack -r example/Rush/resources.rs -o build/shaderpack -p shaders.sqz
//      ShaderPack -t linux -r example/Rush/resources.rs -o build/shaderpack -p shaders_linux.sqz
//
//  A report (<folder>/<platform>.txt): each target, its effects, its shaders (the programs: an
//  entry, vertex, fragment or compute, and not a library: "#pragma once"), the failures, the
//  translations of each language.
//
#define SQUARE_MAIN
#include <Square/Square.h>
#include <filesystem>
#include <regex>
#include <set>
#include <sstream>

static Square::Shell::ParserCommands s_ShellCommands
{
      Square::Shell::Command{ "platform",  "t", "the platform of the pack [windows, macos, ios, linux, android] (default: this one)", Square::Shell::ValueType::value_string, false, Square::Shell::Value_t(std::string("")) }
    , Square::Shell::Command{ "resources", "r", "the resource files (.rs) of the effects, by ',' (common: always)"                , Square::Shell::ValueType::value_string, false, Square::Shell::Value_t(std::string("")) }
    , Square::Shell::Command{ "output",    "o", "the folder of the translations (by language)"                                   , Square::Shell::ValueType::value_string, true,  Square::Shell::Value_t() }
    , Square::Shell::Command{ "pack",      "p", "the archive of the folder made at the end (shaders.sqz)"                        , Square::Shell::ValueType::value_string, false, Square::Shell::Value_t(std::string("")) }
    , Square::Shell::Command{ "gamma",     "g", "the gamma corrections of the game [all, on, off]"                               , Square::Shell::ValueType::value_string, false, Square::Shell::Value_t(std::string("all")) }
    , Square::Shell::Command{ "backend",   "b", "the backend started (any: nothing compiled by it) [ogl, d3d, mtl]"              , Square::Shell::ValueType::value_string, false, Square::Shell::Value_t(std::string("ogl")) }
    , Square::Shell::Command{ "help",      "h", "show help"                                                                      , Square::Shell::ValueType::value_none  , false, Square::Shell::Value_t(false) }
};

namespace AuxShaderPack
{
    //a target in the report: its language, its driver, its gamma correction
    static std::string describe(const Square::Resource::ShaderTarget& target)
    {
        std::string out = target.name();
        out += " " + std::to_string(target.m_version);
        out += target.m_info.m_geometry_shader ? " geometry" : " no-geometry";
        out += target.m_info.m_vertex_viewport_index ? " vertex-layer" : " no-vertex-layer";
        if (target.m_gamma)
        {
            out += target.m_srgb_target ? " gamma(framebuffer)" : " gamma(shader)";
        }
        else
        {
            out += " no-gamma";
        }
        return out;
    }

    //the gamma corrections asked ("all", "on", "off")
    static bool gamma_asked(const std::string& gamma, const Square::Resource::ShaderTarget& target)
    {
        bool asked = true;
        if (gamma == "on")
        {
            asked = target.m_gamma;
        }
        else if (gamma == "off")
        {
            asked = !target.m_gamma;
        }
        return asked;
    }
}

class ShaderPackTool : public Square::AppInterface
{
public:

    ShaderPackTool
    (
          const std::string& platform
        , const std::string& gamma
        , const std::vector<std::string>& resources
        , const std::string& output
        , const std::string& pack
    )
    : m_platform(platform)
    , m_gamma(gamma)
    , m_resources(resources)
    , m_output(output)
    , m_pack(pack)
    {
    }

    void start() override
    {
        using namespace Square;
        auto render_system = System::get<RenderSystem>(context());
        if (!render_system || !render_system->render())
        {
            context().logger()->error("ShaderPack: no render");
            return;
        }
        //the targets of the platform (the gamma corrections asked)
        std::vector<Resource::ShaderTarget> targets;
        for (const Resource::ShaderTarget& target : Resource::ShaderTarget::of_platform(m_platform))
        {
            if (AuxShaderPack::gamma_asked(m_gamma, target))
            {
                targets.push_back(target);
            }
        }
        if (targets.empty())
        {
            context().logger()->error("ShaderPack: no target for the platform '" + m_platform + "' (gamma: " + m_gamma + ")");
            return;
        }
        //the folders of its languages made again (their old translations gone)
        std::set<std::string> languages;
        for (const Resource::ShaderTarget& target : targets)
        {
            languages.insert(target.name());
        }
        std::error_code error;
        for (const std::string& language : languages)
        {
            std::filesystem::remove_all(std::filesystem::u8path(Filesystem::join(m_output, language)), error);
        }
        std::filesystem::create_directories(std::filesystem::u8path(m_output), error);
        //the translations into it (the pack of the engine not read: every shader translated; the
        //same translation of two targets: found there)
        render_system->shader_cache().folder(m_output);
        render_system->shader_cache().pack(std::string());
        //the resources: common, then the ones given
        context().add_resources(Filesystem::join(Filesystem::resource_dir(), "common/common.rs"));
        for (const std::string& file : m_resources)
        {
            context().add_resources(Filesystem::join(Filesystem::resource_dir(), file));
        }
        const std::vector<std::string> effects = context().resource_names<Resource::Effect>();
        const std::vector<std::string> shaders = programs();
        //each target: every effect, every shader (new instances: none kept from another target)
        std::ostringstream report;
        size_t failures = 0;
        const Resource::ShaderTarget driver = render_system->shader_target();
        for (const Resource::ShaderTarget& target : targets)
        {
            render_system->shader_target(target);
            std::ostringstream failed;
            size_t loaded = 0;
            for (const std::string& name : effects)
            {
                const bool ok = context().resource_instance<Resource::Effect>(name) != nullptr;
                loaded += ok ? 1 : 0;
                failed << (ok ? "" : "    FAILED effect " + name + "\n");
                failures += ok ? 0 : 1;
            }
            for (const std::string& name : shaders)
            {
                const bool ok = context().resource_instance<Resource::Shader>(name) != nullptr;
                loaded += ok ? 1 : 0;
                failed << (ok ? "" : "    FAILED shader " + name + "\n");
                failures += ok ? 0 : 1;
            }
            report << AuxShaderPack::describe(target) << ": " << loaded << " of " << (effects.size() + shaders.size()) << "\n" << failed.str();
            context().logger()->info("ShaderPack: " + AuxShaderPack::describe(target) + " done");
        }
        render_system->shader_target(driver);
        //the translations made, by language
        report << "\n";
        for (const std::string& language : languages)
        {
            size_t translations = 0;
            for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::u8path(Filesystem::join(m_output, language)), error))
            {
                translations += entry.path().extension() == ".sqsh" ? 1 : 0;
            }
            report << language << ": " << translations << " translations\n";
        }
        report << failures << " failures\n";
        //the archive of the folder (every language in it)
        if (!m_pack.empty())
        {
            const bool packed = Filesystem::archive_write(m_output, m_pack);
            report << (packed ? "packed in " : "FAILED to pack in ") << m_pack << "\n";
        }
        Filesystem::text_file_write_all(Filesystem::join(m_output, m_platform + ".txt"), report.str());
        context().logger()->info(report.str());
    }

    bool run(double) override { return false; }
    bool end() override { return true; }

private:

    std::string              m_platform;
    std::string              m_gamma;
    std::vector<std::string> m_resources;
    std::string              m_output;
    std::string              m_pack;

    //the shaders that are programs: an entry, not a library to include ("#pragma once")
    std::vector<std::string> programs()
    {
        using namespace Square;
        const std::regex entry(R"(\b(vertex|fragment|compute)\s*\()");
        const std::regex library(R"(#\s*pragma\s+once)");
        std::vector<std::string> out;
        for (const std::string& name : context().resource_names<Resource::Shader>())
        {
            const std::string source = Filesystem::text_file_read_all(context().resource_path<Resource::Shader>(name));
            if (std::regex_search(source, entry) && !std::regex_search(source, library))
            {
                out.push_back(name);
            }
        }
        return out;
    }
};

square_main(s_ShellCommands)(Square::Application& app, Square::Shell::ParserValue& args, Square::Shell::Error& errors)
{
    using namespace Square;
    if (args.find("help") != args.end() && std::get<bool>(args["help"]))
    {
        app.context()->logger()->info(Shell::filename(args) + ":\n");
        app.context()->logger()->info(Shell::help(s_ShellCommands));
        return 0;
    }
    if (errors.type != Shell::ErrorType::none)
    {
        app.context()->logger()->error("Error to parse input [" + std::to_string(errors.id_argument) + "]: " + errors.what);
        return -1;
    }
    //the resource files, by ','
    std::vector<std::string> resources;
    std::stringstream list(std::get<std::string>(args.at("resources")));
    for (std::string file; std::getline(list, file, ',');)
    {
        if (!file.empty())
        {
            resources.push_back(file);
        }
    }
    std::string platform = std::get<std::string>(args.at("platform"));
    if (platform.empty())
    {
        platform = Resource::ShaderTarget::host_platform();
    }
    //the backend started (it compiles nothing: the targets are translated only)
    const std::string backend = std::get<std::string>(args.at("backend"));
    const WindowRenderDriver render_driver = backend == "d3d"
        ? WindowRenderDriver{ Render::RenderDriver::DR_DIRECTX, 11, 0, 24, 8, GpuType::GPU_HIGH, true, false }
        : backend == "mtl"
        ? WindowRenderDriver{ Render::RenderDriver::DR_METAL, 3, 0, 24, 8, GpuType::GPU_HIGH, true, false }
        : WindowRenderDriver{ Render::RenderDriver::DR_OPENGL, 4, 1, 24, 8, GpuType::GPU_HIGH, true, false };
    //a tool: no splash screen
    app.splash_screen(false);
    app.execute
    (
          WindowSizePixel({ 640, 480 })
        , WindowMode::NOT_RESIZABLE
        , render_driver
        , "ShaderPack"
        , new ShaderPackTool
        (
              platform
            , std::get<std::string>(args.at("gamma"))
            , resources
            , std::get<std::string>(args.at("output"))
            , std::get<std::string>(args.at("pack"))
        )
    );
    return 0;
}
