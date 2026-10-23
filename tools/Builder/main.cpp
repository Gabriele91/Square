//
//  Builder
//  The builds of a game of Square: cmake and ninja, the compiler of a platform (or its SDK), a
//  configuration (Debug, Release, Retail) of a target; its assets (the release of the platform:
//  AssetPack, ShaderPack), its deploy on a mobile, its run. A command line, else a window (the UI
//  of the engine, RmlUi):
//
//      Builder                                              the window
//      Builder -s                                           the tools found
//      Builder -g . -t Rush -p windows -c Release           a build
//      Builder -g . -t Rush -p android -c Retail -a -d -r   a release on the device, started
//      Builder -p android -c Debug -n                       its steps, not run
//
//  What was asked last is kept (<app data>/square/builder.json): the next one starts from it.
//
#define SQUARE_MAIN
#include <Square/Square.h>
#include <cstdio>
#include <iostream>
#include <thread>
#include "Build.h"
#include "Process.h"
#include "Settings.h"
#include "Toolchain.h"

static Square::Shell::ParserCommands s_ShellCommands
{
      Square::Shell::Command{ "game",     "g", "the folder of the game (its CMakeLists.txt)"                     , Square::Shell::ValueType::value_string, false, Square::Shell::Value_t(Square::String("")) }
    , Square::Shell::Command{ "target",   "t", "the target of the game (Rush)"                                   , Square::Shell::ValueType::value_string, false, Square::Shell::Value_t(Square::String("")) }
    , Square::Shell::Command{ "platform", "p", "the platform [windows, linux, macos, android] (a build: the command line)", Square::Shell::ValueType::value_string, false, Square::Shell::Value_t(Square::String("")) }
    , Square::Shell::Command{ "config",   "c", "the configuration [Debug, Release, Retail]"                      , Square::Shell::ValueType::value_string, false, Square::Shell::Value_t(Square::String("")) }
    , Square::Shell::Command{ "compiler", "x", "the compiler [msvc, clang-cl; gcc, clang; ndk] (default: the first found)", Square::Shell::ValueType::value_string, false, Square::Shell::Value_t(Square::String("")) }
    , Square::Shell::Command{ "output",   "o", "the folder of the builds (default: <game>/builds)"               , Square::Shell::ValueType::value_string, false, Square::Shell::Value_t(Square::String("")) }
    , Square::Shell::Command{ "engine",   "e", "the folder of the engine (default: the one of the game)"         , Square::Shell::ValueType::value_string, false, Square::Shell::Value_t(Square::String("")) }
    , Square::Shell::Command{ "assets",   "a", "the release of the platform: the archives, the shaders packed"   , Square::Shell::ValueType::value_none,   false, Square::Shell::Value_t(false) }
    , Square::Shell::Command{ "deploy",   "d", "a mobile: the APK installed, its resources pushed"               , Square::Shell::ValueType::value_none,   false, Square::Shell::Value_t(false) }
    , Square::Shell::Command{ "run",      "r", "the game started (on the device, on the desktop)"                , Square::Shell::ValueType::value_none,   false, Square::Shell::Value_t(false) }
    , Square::Shell::Command{ "plan",     "n", "the steps of the build, not run"                                 , Square::Shell::ValueType::value_none,   false, Square::Shell::Value_t(false) }
    , Square::Shell::Command{ "detect",   "s", "the tools found (cmake, ninja, the compilers, the SDKs)"         , Square::Shell::ValueType::value_none,   false, Square::Shell::Value_t(false) }
    , Square::Shell::Command{ "cmake",    "m", "cmake (kept in the settings)"                                    , Square::Shell::ValueType::value_string, false, Square::Shell::Value_t(Square::String("")) }
    , Square::Shell::Command{ "ninja",    "j", "ninja (kept in the settings)"                                    , Square::Shell::ValueType::value_string, false, Square::Shell::Value_t(Square::String("")) }
    , Square::Shell::Command{ "sdk",      "k", "the Android SDK (kept in the settings)"                          , Square::Shell::ValueType::value_string, false, Square::Shell::Value_t(Square::String("")) }
    , Square::Shell::Command{ "jdk",      "v", "the JDK 17 of Gradle (kept in the settings)"                     , Square::Shell::ValueType::value_string, false, Square::Shell::Value_t(Square::String("")) }
    , Square::Shell::Command{ "backend",  "b", "the window: its render [vk, ogl, d3d, mtl]"                      , Square::Shell::ValueType::value_string, false, Square::Shell::Value_t(Square::String("")) }
    , Square::Shell::Command{ "help",     "h", "show help"                                                       , Square::Shell::ValueType::value_none,   false, Square::Shell::Value_t(false) }
};

namespace AuxBuilder
{
    //a line of the log on the console
    static void print(const Square::String& line)
    {
        std::cout << line << std::endl;
    }

    //a text of the command line given: the value, else the one kept
    static void text_of(Square::Shell::ParserValue& args, const char* name, Square::String& value)
    {
        const Square::String given = std::get<Square::String>(args.at(name));
        if (!given.empty())
        {
            value = given;
        }
    }

    //the window: its render by name
    struct Backend
    {
        const char*                m_name;
        Square::WindowRenderDriver m_driver;
    };
    static const Backend s_backends[] =
    {
        { "vk",  { Square::Render::RenderDriver::DR_VULKAN,  1, 1, 24, 8, Square::GpuType::GPU_HIGH, true, false } },
        { "ogl", { Square::Render::RenderDriver::DR_OPENGL,  4, 1, 24, 8, Square::GpuType::GPU_HIGH, true, false } },
        { "d3d", { Square::Render::RenderDriver::DR_DIRECTX, 11, 0, 24, 8, Square::GpuType::GPU_HIGH, true, false } },
        { "mtl", { Square::Render::RenderDriver::DR_METAL,   3, 0, 24, 8, Square::GpuType::GPU_HIGH, true, false } },
    };

    static Square::WindowRenderDriver backend(Square::String name)
    {
        if (name.empty())
        {
        #if defined(__APPLE__)
            name = "mtl";
        #else
            name = "vk";
        #endif
        }
        for (const Backend& backend : s_backends)
        {
            if (name == backend.m_name)
            {
                return backend.m_driver;
            }
        }
        return s_backends[0].m_driver;
    }

    //the lines of the log the window keeps
    static constexpr size_t s_log_lines = 400;
}

//the window: the fields of a build, its steps run by a thread of their own, the log of it
class BuilderWindow : public Square::AppInterface
{
public:

    BuilderWindow(const Builder::Settings& settings)
    : m_settings(settings)
    {
    }

    void start() override
    {
        using namespace Square;
        context().engine_dir(SQUARE_BUILDER_ENGINE_DIR);
        context().add_resources(context().engine_path("common/common.rs"));
        auto* ui_system = System::get<UISystem>(context());
        if (!ui_system || !ui_system->ui().valid())
        {
            context().logger()->error("Builder: no UI");
            return;
        }
        unsigned int width = 0;
        unsigned int height = 0;
        window().get_size(width, height);
        ui_system->reference_size({ int(width), int(height) });
        UI::Context::load_font(context().engine_path("common/ui/LatoLatin-Regular.ttf"));
        UI::Context::load_font(context().engine_path("common/ui/LatoLatin-Bold.ttf"));
        //the tools first (the compilers of the document)
        m_toolchain.detect(m_settings, [this](const Square::String& line) { push(line); });
        fields_of_settings();
        fields_of_toolchain();
        //the model, then the document
        m_model = ui_system->ui().create_data_model("builder");
        for (const Text& text : texts())
        {
            m_model.bind(text.m_name, text.m_value);
        }
        for (const Flag& flag : flags())
        {
            m_model.bind(flag.m_name, flag.m_value);
        }
        m_model.bind("running", &m_running_shown);
        m_model.bind("log", &m_log);
        m_model.bind("status", &m_status);
        m_model.bind_event("build", [this](UI::Event&) { start_work(false); });
        m_model.bind_event("plan", [this](UI::Event&) { start_work(true); });
        m_model.bind_event("stop", [this](UI::Event&) { m_process.stop(); });
        m_model.bind_event("detect", [this](UI::Event&) { start_detect(); });
        m_model.bind_event("clear", [this](UI::Event&) { m_lines.clear(); m_log.clear(); });
        ui_system->watch(m_model);
        m_document = ui_system->ui().load_from_memory(document());
        m_document.show();
        m_status = "ready";
        m_scroll = true;
    }

    bool run(double) override
    {
        using namespace Square;
        //the lines of the thread, the end of its work
        Vector<String> lines;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            lines.swap(m_pending);
        }
        for (const String& line : lines)
        {
            m_lines.push_back(line);
        }
        if (!lines.empty() || m_lines.size() > AuxBuilder::s_log_lines)
        {
            if (m_lines.size() > AuxBuilder::s_log_lines)
            {
                m_lines.erase(m_lines.begin(), m_lines.end() - AuxBuilder::s_log_lines);
            }
            m_log.clear();
            for (const String& line : m_lines)
            {
                m_log += line;
                m_log += '\n';
            }
            m_scroll = true;
        }
        if (!m_running && m_worker.joinable())
        {
            m_worker.join();
            std::lock_guard<std::mutex> lock(m_mutex);
            m_status = m_result;
        }
        m_running_shown = m_running;
        //the tools found again: in their fields
        if (m_detected.exchange(false))
        {
            fields_of_toolchain();
        }
        //another platform: its first compiler
        if (m_platform != m_platform_shown)
        {
            m_platform_shown = m_platform;
            const Vector<String> compilers = m_toolchain.compilers(m_platform);
            const bool known = std::find(compilers.begin(), compilers.end(), m_compiler) != compilers.end();
            if (!known)
            {
                m_compiler.clear();
                if (!compilers.empty())
                {
                    m_compiler = compilers[0];
                }
            }
        }
        //the log at its end (once laid out: its height known the next frame)
        if (m_scroll && m_document.valid())
        {
            UI::Element log = m_document.find("log");
            if (log.valid())
            {
                log.scroll_top(log.scroll_height());
            }
            m_scroll = m_scroll_frames++ < 1;
            if (!m_scroll)
            {
                m_scroll_frames = 0;
            }
        }
        return true;
    }

    bool end() override
    {
        m_process.stop();
        if (m_worker.joinable())
        {
            m_worker.join();
        }
        settings_of_fields();
        m_settings.save();
        return true;
    }

private:

    //a field of the window: its name in the model, its value
    struct Text
    {
        const char*     m_name;
        Square::String* m_value;
    };
    struct Flag
    {
        const char* m_name;
        bool*       m_value;
    };

    Builder::Settings          m_settings;
    Builder::Toolchain         m_toolchain;
    Builder::Process           m_process;
    Square::UI::DataModel      m_model;
    Square::UI::Document       m_document;
    //the fields
    Square::String m_game;
    Square::String m_target;
    Square::String m_engine;
    Square::String m_build_root;
    Square::String m_platform;
    Square::String m_platform_shown;
    Square::String m_config;
    Square::String m_compiler;
    Square::String m_cmake;
    Square::String m_ninja;
    Square::String m_android_sdk;
    Square::String m_java_home;
    bool           m_assets{ false };
    bool           m_deploy{ false };
    bool           m_run{ false };
    //the log, the state of the work
    Square::Vector<Square::String> m_lines;
    Square::String               m_log;
    Square::String               m_status;
    bool                         m_running_shown{ false };
    bool                         m_scroll{ false };
    int                          m_scroll_frames{ 0 };
    //the thread of the work: its lines, its end
    std::thread                    m_worker;
    std::mutex                     m_mutex;
    std::atomic<bool>              m_running{ false };
    std::atomic<bool>              m_detected{ false };
    Square::Vector<Square::String> m_pending;
    Square::String                 m_result;

    Square::Vector<Text> texts()
    {
        return
        {
            { "game",        &m_game },
            { "target",      &m_target },
            { "engine",      &m_engine },
            { "build_root",  &m_build_root },
            { "platform",    &m_platform },
            { "config",      &m_config },
            { "compiler",    &m_compiler },
            { "cmake",       &m_cmake },
            { "ninja",       &m_ninja },
            { "android_sdk", &m_android_sdk },
            { "java_home",   &m_java_home },
        };
    }

    Square::Vector<Flag> flags()
    {
        return
        {
            { "assets", &m_assets },
            { "deploy", &m_deploy },
            { "run",    &m_run },
        };
    }

    void fields_of_settings()
    {
        const Builder::Request request = Builder::Request::from(m_settings);
        m_game = request.m_game;
        m_target = request.m_target;
        m_engine = request.m_engine;
        m_build_root = request.m_build_root;
        m_platform = request.m_platform;
        if (m_platform.empty())
        {
            m_platform = Builder::Toolchain::host();
        }
        m_config = request.m_config;
        m_compiler = request.m_compiler;
        m_assets = request.m_assets;
        m_deploy = request.m_deploy;
        m_run = request.m_run;
        m_cmake = m_settings.m_cmake;
        m_ninja = m_settings.m_ninja;
        m_android_sdk = m_settings.m_android_sdk;
        m_java_home = m_settings.m_java_home;
    }

    //the tools found in their fields (the ones given first: Toolchain::detect checks them, a
    //folder moved found again)
    void fields_of_toolchain()
    {
        m_cmake = m_toolchain.cmake();
        m_ninja = m_toolchain.ninja();
        m_android_sdk = m_toolchain.android_sdk();
        m_java_home = m_toolchain.java_home();
    }

    void settings_of_fields()
    {
        request().to(m_settings);
        m_settings.m_cmake = m_cmake;
        m_settings.m_ninja = m_ninja;
        m_settings.m_android_sdk = m_android_sdk;
        m_settings.m_java_home = m_java_home;
    }

    Builder::Request request() const
    {
        Builder::Request out;
        out.m_game = m_game;
        out.m_target = m_target;
        out.m_engine = m_engine;
        out.m_build_root = m_build_root;
        out.m_platform = m_platform;
        out.m_config = m_config;
        out.m_compiler = m_compiler;
        out.m_assets = m_assets;
        out.m_deploy = m_deploy;
        out.m_run = m_run;
        return out;
    }

    //a line of the thread (shown by the next frame)
    void push(const Square::String& line)
    {
        AuxBuilder::print(line);
        std::lock_guard<std::mutex> lock(m_mutex);
        m_pending.push_back(line);
    }

    //a work of the thread (one at a time): its status shown while it runs, then its result
    void work(const Square::String& status, const std::function<Square::String()>& job)
    {
        if (m_running)
        {
            return;
        }
        if (m_worker.joinable())
        {
            m_worker.join();
        }
        settings_of_fields();
        m_settings.save();
        m_status = status;
        m_running = true;
        m_running_shown = true;
        m_process.reset();
        m_worker = std::thread([this, job]()
        {
            const Square::String result = job();
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_result = result;
            }
            m_running = false;
        });
    }

    //a build (plan: its steps only)
    void start_work(bool plan)
    {
        const Builder::Request asked = request();
        Square::String status = "building";
        if (plan)
        {
            status = "planning";
        }
        work(status, [this, asked, plan]() -> Square::String
        {
            const Builder::LineCallback log = [this](const Square::String& line) { push(line); };
            Builder::Request build = asked;
            Square::Vector<Builder::Step> steps;
            if (!Builder::Build::plan(build, m_toolchain, steps, log))
            {
                return "cannot build";
            }
            if (plan)
            {
                Builder::Build::print(steps, log);
                return "planned";
            }
            if (!Builder::Build::run(steps, m_process, log))
            {
                if (m_process.stopped())
                {
                    return "stopped";
                }
                return "failed";
            }
            return "done";
        });
    }

    //the tools found again (the ones of the fields first)
    void start_detect()
    {
        work("detecting", [this]() -> Square::String
        {
            m_toolchain.detect(m_settings, [this](const Square::String& line) { push(line); });
            m_detected = true;
            return "ready";
        });
    }

    //the document: the fields, the radios of the platforms, of the configurations, of the
    //compilers found (each shown with its platform), the log
    Square::String document()
    {
        using namespace Square;
        String platforms;
        String compilers;
        for (const Builder::Platform& platform : Builder::Toolchain::platforms())
        {
            const bool here = !platform.m_host || Builder::Toolchain::host() == platform.m_host;
            if (!here)
            {
                continue;
            }
            const String name = platform.m_name;
            platforms += "<label class=\"choice\"><input type=\"radio\" name=\"platform\" value=\"" + name + "\" data-checked=\"platform\"/> " + name + "</label>";
            compilers += "<span data-if=\"platform == '" + name + "'\">";
            const Vector<String> found = m_toolchain.compilers(name);
            for (const String& compiler : found)
            {
                compilers += "<label class=\"choice\"><input type=\"radio\" name=\"compiler\" value=\"" + compiler + "\" data-checked=\"compiler\"/> " + compiler + "</label>";
            }
            if (found.empty())
            {
                compilers += "<span class=\"hint\">none found</span>";
            }
            compilers += "</span>";
        }
        String configs;
        for (const String& config : Builder::Build::configs())
        {
            configs += "<label class=\"choice\"><input type=\"radio\" name=\"config\" value=\"" + config + "\" data-checked=\"config\"/> " + config + "</label>";
        }
        const String style = context().engine_path("common/ui/default.rcss");
        return
            "<rml><head><title>Builder</title>"
            "<link type=\"text/rcss\" href=\"" + style + "\"/>"
            "<style>"
            "body { width: 100%; height: 100%; padding: 12dp; box-sizing: border-box; background-color: #0f141a; display: flex; flex-direction: column; }"
            ".row { display: flex; flex-direction: row; align-items: center; margin-bottom: 6dp; }"
            ".label { width: 110dp; color: #9aa6b6; }"
            ".row input.text { flex: 1; }"
            ".choice { margin-right: 16dp; }"
            ".hint { color: #7a8494; }"
            ".section { color: #5aa0ff; font-weight: bold; margin: 8dp 0 6dp 0; }"
            ".buttons button { margin-right: 8dp; }"
            ".status { margin-left: 8dp; color: #e8a33c; }"
            "#log { flex: 1; margin-top: 8dp; padding: 6dp; background-color: #080b0f; overflow-y: auto; white-space: pre-wrap; font-size: 13dp; color: #c8d0dc; }"
            "</style></head>"
            "<body data-model=\"builder\">"
            "<div class=\"section\">Build</div>"
            "<div class=\"row\"><span class=\"label\">Game</span><input type=\"text\" data-value=\"game\"/></div>"
            "<div class=\"row\"><span class=\"label\">Target</span><input type=\"text\" data-value=\"target\"/></div>"
            "<div class=\"row\"><span class=\"label\">Builds</span><input type=\"text\" data-value=\"build_root\"/></div>"
            "<div class=\"row\"><span class=\"label\">Engine</span><input type=\"text\" data-value=\"engine\"/></div>"
            "<div class=\"row\"><span class=\"label\">Platform</span>" + platforms + "</div>"
            "<div class=\"row\"><span class=\"label\">Compiler</span>" + compilers + "</div>"
            "<div class=\"row\"><span class=\"label\">Config</span>" + configs + "</div>"
            "<div class=\"row\"><span class=\"label\">Steps</span>"
            "<label class=\"choice\"><input type=\"checkbox\" data-checked=\"assets\"/> assets (release)</label>"
            "<label class=\"choice\"><input type=\"checkbox\" data-checked=\"deploy\"/> deploy (mobile)</label>"
            "<label class=\"choice\"><input type=\"checkbox\" data-checked=\"run\"/> run</label>"
            "</div>"
            "<div class=\"section\">Tools (Detect: the ones found; empty: found again)</div>"
            "<div class=\"row\"><span class=\"label\">cmake</span><input type=\"text\" data-value=\"cmake\"/></div>"
            "<div class=\"row\"><span class=\"label\">ninja</span><input type=\"text\" data-value=\"ninja\"/></div>"
            "<div class=\"row\"><span class=\"label\">Android SDK</span><input type=\"text\" data-value=\"android_sdk\"/></div>"
            "<div class=\"row\"><span class=\"label\">JDK 17</span><input type=\"text\" data-value=\"java_home\"/></div>"
            "<div class=\"row buttons\">"
            "<button data-if=\"!running\" data-event-click=\"build\">Build</button>"
            "<button data-if=\"!running\" data-event-click=\"plan\">Plan</button>"
            "<button data-if=\"!running\" data-event-click=\"detect\">Detect</button>"
            "<button data-if=\"running\" data-event-click=\"stop\">Stop</button>"
            "<button data-event-click=\"clear\">Clear</button>"
            "<span class=\"status\">{{status}}</span>"
            "</div>"
            "<div id=\"log\">{{log}}</div>"
            "</body></rml>";
    }
};

square_main(s_ShellCommands)(Square::Application& app, Square::Shell::ParserValue& args, Square::Shell::Error& errors)
{
    using namespace Square;
    //(the log of the engine at once: the lines of the commands between them, none lost)
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    if (args.find("help") != args.end() && std::get<bool>(args["help"]))
    {
        AuxBuilder::print(Shell::filename(args) + ":\n");
        AuxBuilder::print(Shell::help(s_ShellCommands));
        return 0;
    }
    if (errors.type != Shell::ErrorType::none)
    {
        AuxBuilder::print("Error to parse input [" + Square::to_string(errors.id_argument) + "]: " + errors.what);
        return -1;
    }
    //the settings kept, the tools given (kept with them)
    Builder::Settings settings;
    settings.load();
    AuxBuilder::text_of(args, "cmake", settings.m_cmake);
    AuxBuilder::text_of(args, "ninja", settings.m_ninja);
    AuxBuilder::text_of(args, "sdk", settings.m_android_sdk);
    AuxBuilder::text_of(args, "jdk", settings.m_java_home);
    const bool detect = std::get<bool>(args.at("detect"));
    const bool plan = std::get<bool>(args.at("plan"));
    const bool build = !std::get<String>(args.at("platform")).empty();
    //the window: no build asked
    if (!detect && !plan && !build)
    {
        app.splash_screen(false);
        app.execute
        (
              WindowSizePixel({ 1100, 800 })
            , WindowMode::RESIZABLE
            , AuxBuilder::backend(std::get<String>(args.at("backend")))
            , "Square Builder"
            , new BuilderWindow(settings)
        );
        return 0;
    }
    //the command line
    Builder::Toolchain toolchain;
    Builder::LineCallback log = AuxBuilder::print;
    Builder::LineCallback silent;
    if (detect)
    {
        toolchain.detect(settings, log);
    }
    else
    {
        toolchain.detect(settings, silent);
    }
    if (!build && !plan)
    {
        settings.save();
        return 0;
    }
    AuxBuilder::text_of(args, "game", settings.m_game);
    AuxBuilder::text_of(args, "target", settings.m_target);
    AuxBuilder::text_of(args, "platform", settings.m_platform);
    AuxBuilder::text_of(args, "config", settings.m_config);
    AuxBuilder::text_of(args, "compiler", settings.m_compiler);
    AuxBuilder::text_of(args, "output", settings.m_build_root);
    AuxBuilder::text_of(args, "engine", settings.m_engine);
    settings.m_assets = std::get<bool>(args.at("assets"));
    settings.m_deploy = std::get<bool>(args.at("deploy"));
    settings.m_run = std::get<bool>(args.at("run"));
    if (settings.m_game.empty())
    {
        settings.m_game = Filesystem::working_dir();
    }
    Builder::Request request = Builder::Request::from(settings);
    Square::Vector<Builder::Step> steps;
    if (!Builder::Build::plan(request, toolchain, steps, log))
    {
        return 1;
    }
    request.to(settings);
    settings.save();
    if (plan)
    {
        Builder::Build::print(steps, log);
        return 0;
    }
    Builder::Process process;
    if (!Builder::Build::run(steps, process, log))
    {
        return 1;
    }
    return 0;
}
