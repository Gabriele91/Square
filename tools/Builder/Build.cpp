//
//  Build.cpp
//  Builder
//
//  See Build.h.
//
#include "Build.h"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <regex>

namespace Builder
{
    namespace AuxBuild
    {
        static const Square::Vector<Square::String> s_configs{ "Debug", "Release", "Retail" };

        //the drivers of the desktop of a host (loaded when they run: by the game, by ShaderPack)
        struct HostDrivers
        {
            const char*                    m_host;
            Square::Vector<Square::String> m_drivers;
        };
        static const HostDrivers s_drivers[] =
        {
            { "windows", { "SqDX11", "SqVulkan", "SqOpenGL" } },
            { "linux",   { "SqVulkan", "SqOpenGL" } },
            { "macos",   { "SqMetal", "SqOpenGL" } },
        };

        //the tools of the release of a platform, built by the host
        static const Square::Vector<Square::String> s_tools{ "AssetPack", "ShaderPack" };

        //a text of the standard library (std::filesystem, std::regex) in one of the engine
        static Square::String text_of(const std::string& text)
        {
            return Square::String(text.c_str(), text.size());
        }

        //two paths joined, with '/' (the one of cmake: no path written in two ways)
        static Square::String join(const Square::String& left, const Square::String& right)
        {
            Square::String out = Square::Filesystem::join(left, right);
            std::replace(out.begin(), out.end(), '\\', '/');
            return out;
        }

        static Square::String lower(Square::String text)
        {
            std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return char(std::tolower(c)); });
            return text;
        }

        //a path absolute, normal, with '/'
        static Square::String absolute(const Square::String& path)
        {
            std::error_code error;
            const std::filesystem::path full = std::filesystem::absolute(std::filesystem::u8path(path), error);
            return text_of(full.lexically_normal().generic_u8string());
        }

        static Square::Vector<Square::String> drivers(const Square::String& host)
        {
            for (const HostDrivers& entry : s_drivers)
            {
                if (host == entry.m_host)
                {
                    return entry.m_drivers;
                }
            }
            return {};
        }

        //the files of assets/ out of the release of a game: its release.cmake, set(EXCLUDE ...)
        static Square::Vector<Square::String> excluded(const Square::String& game)
        {
            Square::Vector<Square::String> out;
            const Square::String file = Square::Filesystem::join(game, "release.cmake");
            if (!Square::Filesystem::is_file(file))
            {
                return out;
            }
            //(its comments out first)
            const std::string source = Square::Filesystem::text_file_read_all(file).c_str();
            const std::string text = std::regex_replace(source, std::regex("#[^\n]*"), "");
            std::smatch match;
            if (!std::regex_search(text, match, std::regex("set\\s*\\(\\s*EXCLUDE([^)]*)\\)")))
            {
                return out;
            }
            Square::StringStream values(text_of(match[1].str()));
            for (Square::String value; values >> value;)
            {
                out.push_back(value);
            }
            return out;
        }

        //the archives of a game in its release (assets/*.sqz but the excluded ones), by ','
        static Square::String archives(const Square::String& game, const Square::Vector<Square::String>& exclude)
        {
            Square::Vector<Square::String> files;
            std::error_code error;
            const std::filesystem::path assets = std::filesystem::u8path(Square::Filesystem::join(game, "assets"));
            for (const auto& entry : std::filesystem::directory_iterator(assets, error))
            {
                const Square::String name = text_of(entry.path().filename().u8string());
                const bool archive = entry.is_regular_file() && Square::Filesystem::get_extension(name) == ".sqz";
                const bool out = std::find(exclude.begin(), exclude.end(), name) != exclude.end();
                if (archive && !out)
                {
                    files.push_back(text_of(entry.path().generic_u8string()));
                }
            }
            std::sort(files.begin(), files.end());
            Square::String list;
            for (const Square::String& file : files)
            {
                if (!list.empty())
                {
                    list += ',';
                }
                list += file;
            }
            return list;
        }

        //the components of the installation of a build but the one of no component (the files of
        //the game, not its libraries: the ones of a device)
        static Square::Vector<Square::String> components(const Square::String& build)
        {
            Square::Vector<Square::String> out;
            const std::regex component("CMAKE_INSTALL_COMPONENT STREQUAL \"([^\"]+)\"");
            std::error_code error;
            for (const auto& entry : std::filesystem::recursive_directory_iterator(std::filesystem::u8path(build), error))
            {
                if (!entry.is_regular_file() || entry.path().filename() != "cmake_install.cmake")
                {
                    continue;
                }
                const std::string text = Square::Filesystem::text_file_read_all(text_of(entry.path().u8string())).c_str();
                for (std::sregex_iterator it(text.begin(), text.end(), component), end; it != end; ++it)
                {
                    const Square::String name = text_of((*it)[1].str());
                    const bool known = std::find(out.begin(), out.end(), name) != out.end();
                    if (!known && name != "Unspecified")
                    {
                        out.push_back(name);
                    }
                }
            }
            return out;
        }

        //the package of the APK of a build (the applicationId of its Gradle project)
        static Square::String package(const Square::String& build, const Square::String& target)
        {
            const Square::String file = Square::Filesystem::join(build, "android/" + target + "/app/build.gradle.kts");
            const std::string text = Square::Filesystem::text_file_read_all(file).c_str();
            std::smatch match;
            if (std::regex_search(text, match, std::regex("applicationId\\s*=\\s*\"([^\"]+)\"")))
            {
                return text_of(match[1].str());
            }
            return Square::String();
        }

        //the names in a folder (its files, its folders)
        static Square::Vector<Square::String> entries(const Square::String& folder)
        {
            Square::Vector<Square::String> out;
            std::error_code error;
            for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::u8path(folder), error))
            {
                out.push_back(text_of(entry.path().filename().u8string()));
            }
            std::sort(out.begin(), out.end());
            return out;
        }

        //a file newer than another (or the other not there)
        static bool newer(const std::filesystem::path& file, const std::filesystem::path& than)
        {
            std::error_code error;
            if (!std::filesystem::exists(than, error))
            {
                return true;
            }
            return std::filesystem::last_write_time(file, error) > std::filesystem::last_write_time(than, error);
        }

        //the files of a folder of the game newer than the ones of the release (or not there) copied
        //into it (their time kept: adb pushes what is newer than on the device): how many
        static int sync_tree(const std::filesystem::path& from, const std::filesystem::path& to, const LineCallback& log)
        {
            int count = 0;
            std::error_code error;
            for (const auto& entry : std::filesystem::recursive_directory_iterator(from, error))
            {
                if (!entry.is_regular_file())
                {
                    continue;
                }
                const std::filesystem::path target = to / std::filesystem::relative(entry.path(), from, error);
                if (!newer(entry.path(), target))
                {
                    continue;
                }
                std::filesystem::create_directories(target.parent_path(), error);
                std::filesystem::copy_file(entry.path(), target, std::filesystem::copy_options::overwrite_existing, error);
                log("  " + text_of(target.generic_u8string()));
                ++count;
            }
            return count;
        }

        //the newest file under some folders (the sources of the shaders: their effects, their
        //shaders)
        static std::filesystem::file_time_type newest(const Square::Vector<std::filesystem::path>& folders)
        {
            std::filesystem::file_time_type out{};
            std::error_code error;
            for (const std::filesystem::path& folder : folders)
            {
                for (const auto& entry : std::filesystem::recursive_directory_iterator(folder, error))
                {
                    if (entry.is_regular_file())
                    {
                        out = std::max(out, entry.last_write_time(error));
                    }
                }
            }
            return out;
        }

        //the release of a platform as the game is now, without packing again: its files the game
        //changed since (config, effects, the engine's common...) copied again; its packed ones
        //(the archives, shaders.sqz) only told when older than their sources (pack assets again)
        static int refresh_release(const Square::String& game, const Square::String& engine, const Square::String& files, const LineCallback& log)
        {
            namespace fs = std::filesystem;
            const fs::path root = fs::u8path(game);
            const fs::path release = fs::u8path(files);
            std::error_code error;
            int copied = 0;
            Square::Vector<Square::String> stale;
            for (const auto& entry : fs::directory_iterator(release, error))
            {
                const Square::String name = text_of(entry.path().filename().u8string());
                const fs::path source = root / entry.path().filename();
                //(the shaders packed: older than an effect or a shader of the game, of the engine)
                if (name == "shaders.sqz")
                {
                    const Square::Vector<fs::path> sources{ root / "assets/effect", root / "assets/shader", fs::u8path(engine) / "common" };
                    if (newest(sources) > entry.last_write_time(error))
                    {
                        stale.push_back(name);
                    }
                    continue;
                }
                if (entry.is_regular_file())
                {
                    if (fs::exists(source, error) && newer(source, entry.path()))
                    {
                        fs::copy_file(source, entry.path(), fs::copy_options::overwrite_existing, error);
                        log("  " + text_of(entry.path().generic_u8string()));
                        ++copied;
                    }
                    continue;
                }
                //(the engine's common: the one of the engine of the game)
                if (name == "engine")
                {
                    copied += sync_tree(fs::u8path(engine) / "common", entry.path() / "common", log);
                    continue;
                }
                if (name != "assets")
                {
                    if (fs::is_directory(source, error))
                    {
                        copied += sync_tree(source, entry.path(), log);
                    }
                    continue;
                }
                //assets: its folders copied again, its archives (packed) only told
                for (const auto& asset : fs::directory_iterator(entry.path(), error))
                {
                    const fs::path asset_source = source / asset.path().filename();
                    if (asset.is_directory())
                    {
                        if (fs::is_directory(asset_source, error))
                        {
                            copied += sync_tree(asset_source, asset.path(), log);
                        }
                        continue;
                    }
                    if (fs::exists(asset_source, error) && newer(asset_source, asset.path()))
                    {
                        stale.push_back("assets/" + text_of(asset.path().filename().u8string()));
                    }
                }
            }
            log("the release: " + Square::to_string(copied) + " files of the game copied again");
            for (const Square::String& name : stale)
            {
                log("warning: " + name + " is older than its sources: pack assets again to push it");
            }
            return 0;
        }

        //a command of cmake: its arguments, its environment
        static Command cmake(const Toolchain& toolchain, const Square::Vector<Square::String>& arguments, const Environment& environment)
        {
            Command command;
            command.m_arguments.push_back(toolchain.cmake());
            command.m_arguments.insert(command.m_arguments.end(), arguments.begin(), arguments.end());
            command.m_environment = environment;
            return command;
        }

        //the configuration of a build (its compiler only the first time: given again, cmake
        //would throw the cache away when it is written in another way: cl, its full path)
        static Command configure
        (
              const Toolchain& toolchain
            , const Request& request
            , const Platform& platform
            , const Compiler& compiler
            , const Square::String& config
            , const Square::String& folder
            , const Environment& environment
        )
        {
            Square::Vector<Square::String> arguments
            {
                "-S", request.m_game,
                "-B", folder,
                "-G", "Ninja",
                "-DCMAKE_MAKE_PROGRAM=" + toolchain.ninja(),
                "-DCMAKE_BUILD_TYPE=" + config,
                Square::String("-DSQUARE_PLATFORM=") + platform.m_square
            };
            if (!request.m_engine.empty())
            {
                arguments.push_back("-DSQUARE_DIR=" + request.m_engine);
            }
            const bool configured = Square::Filesystem::is_file(Square::Filesystem::join(folder, "CMakeCache.txt"));
            if (!configured && !compiler.m_c.empty())
            {
                arguments.push_back("-DCMAKE_C_COMPILER=" + compiler.m_c);
                arguments.push_back("-DCMAKE_CXX_COMPILER=" + compiler.m_cxx);
            }
            if (platform.m_mobile)
            {
                arguments.push_back("-DSQUARE_ANDROID_JAVA_HOME=" + toolchain.java_home());
            }
            return cmake(toolchain, arguments, environment);
        }

        //the folder of a build: <root>/<platform>-<compiler>-<config>
        static Square::String folder(const Square::String& root, const Square::String& platform, const Square::String& compiler, const Square::String& config)
        {
            return join(root, platform + "-" + compiler + "-" + lower(config));
        }

        //an error of a request: false
        static bool fail(const LineCallback& log, const Square::String& what)
        {
            if (log)
            {
                log("error: " + what);
            }
            return false;
        }

        static Square::String names(const Square::Vector<Square::String>& values)
        {
            Square::String out;
            for (const Square::String& value : values)
            {
                if (!out.empty())
                {
                    out += ", ";
                }
                out += value;
            }
            return out;
        }
    }

    Request Request::from(const Settings& settings)
    {
        Request request;
        request.m_game = settings.m_game;
        request.m_target = settings.m_target;
        request.m_engine = settings.m_engine;
        request.m_build_root = settings.m_build_root;
        request.m_platform = settings.m_platform;
        request.m_config = settings.m_config;
        request.m_compiler = settings.m_compiler;
        request.m_assets = settings.m_assets;
        request.m_deploy = settings.m_deploy;
        request.m_run = settings.m_run;
        return request;
    }

    void Request::to(Settings& settings) const
    {
        settings.m_game = m_game;
        settings.m_target = m_target;
        settings.m_engine = m_engine;
        settings.m_build_root = m_build_root;
        settings.m_platform = m_platform;
        settings.m_config = m_config;
        settings.m_compiler = m_compiler;
        settings.m_assets = m_assets;
        settings.m_deploy = m_deploy;
        settings.m_run = m_run;
    }

    const Square::Vector<Square::String>& Build::configs()
    {
        return AuxBuild::s_configs;
    }

    bool Build::plan(Request& request, Toolchain& toolchain, Square::Vector<Step>& steps, const LineCallback& log)
    {
        using namespace AuxBuild;
        steps.clear();
        //the game, its target
        if (request.m_game.empty())
        {
            return fail(log, "no game (the folder of its CMakeLists.txt)");
        }
        request.m_game = absolute(request.m_game);
        if (!Square::Filesystem::is_file(join(request.m_game, "CMakeLists.txt")))
        {
            return fail(log, "no CMakeLists.txt in " + request.m_game);
        }
        if (request.m_target.empty())
        {
            return fail(log, "no target (the one of the game in its CMakeLists.txt)");
        }
        //the engine: the one asked, else the one of the game, else the one of the tool
        Square::String engine = request.m_engine;
        if (engine.empty() && Square::Filesystem::is_directory(join(request.m_game, "engine/cmake")))
        {
            engine = join(request.m_game, "engine");
        }
    #if defined(SQUARE_BUILDER_ENGINE_DIR)
        if (engine.empty())
        {
            engine = SQUARE_BUILDER_ENGINE_DIR;
        }
    #endif
        if (!request.m_engine.empty())
        {
            request.m_engine = absolute(request.m_engine);
        }
        engine = absolute(engine);
        //the platform, the configuration, the compiler
        if (request.m_platform.empty())
        {
            request.m_platform = Toolchain::host();
        }
        request.m_platform = lower(request.m_platform);
        const Platform* platform = Toolchain::platform(request.m_platform);
        if (!platform)
        {
            return fail(log, "unknown platform " + request.m_platform + " (windows, linux, macos, android)");
        }
        if (platform->m_host && Toolchain::host() != platform->m_host)
        {
            return fail(log, request.m_platform + " is built by its own host, not by " + Toolchain::host());
        }
        if (request.m_config.empty())
        {
            request.m_config = "Release";
        }
        Square::String config;
        for (const Square::String& name : s_configs)
        {
            if (lower(name) == lower(request.m_config))
            {
                config = name;
            }
        }
        if (config.empty())
        {
            return fail(log, "unknown configuration " + request.m_config + " (" + names(s_configs) + ")");
        }
        request.m_config = config;
        const Square::Vector<Square::String> compilers = toolchain.compilers(request.m_platform);
        if (compilers.empty())
        {
            return fail(log, "no compiler of " + request.m_platform + " (Builder --detect)");
        }
        if (request.m_compiler.empty() || !toolchain.compiler(request.m_platform, request.m_compiler))
        {
            //(a compiler of another platform: the one kept of the build before, not told)
            const bool other = !request.m_compiler.empty() && toolchain.known(request.m_compiler);
            if (!request.m_compiler.empty() && !other && log)
            {
                log("compiler " + request.m_compiler + " not found for " + request.m_platform + ": " + compilers[0]);
            }
            request.m_compiler = compilers[0];
        }
        const Compiler& compiler = *toolchain.compiler(request.m_platform, request.m_compiler);
        if (toolchain.cmake().empty() || toolchain.ninja().empty())
        {
            return fail(log, "no cmake or no ninja (the settings: cmake, ninja)");
        }
        Environment environment;
        if (!toolchain.environment(compiler, environment, log))
        {
            return fail(log, "no environment of " + compiler.m_name);
        }
        //the folders
        if (request.m_build_root.empty())
        {
            request.m_build_root = join(request.m_game, "builds");
        }
        request.m_build_root = absolute(request.m_build_root);
        const Square::String& root = request.m_build_root;
        const Square::String build = folder(root, request.m_platform, request.m_compiler, config);
        const Square::String& target = request.m_target;
        //configure, build (a mobile: its APK; the desktop: its drivers too, the game loads them)
        steps.push_back({ "configure", configure(toolchain, request, *platform, compiler, config, build, environment), nullptr });
        Square::Vector<Square::String> build_arguments{ "--build", build, "--target" };
        if (platform->m_mobile)
        {
            build_arguments.push_back(target + "_apk");
        }
        else
        {
            build_arguments.push_back(target);
            for (const Square::String& driver : drivers(Toolchain::host()))
            {
                build_arguments.push_back(driver);
            }
        }
        steps.push_back({ "build", cmake(toolchain, build_arguments, environment), nullptr });
        //the release of the platform: <root>/release/<platform>-<config>
        const Square::String release = join(root, "release/" + request.m_platform + "-" + lower(config));
        Square::String files = release;
        if (platform->m_mobile)
        {
            files = join(release, "files");
        }
        if (request.m_assets)
        {
            //the tools of the host (Release): the build itself when it is that one
            Square::String host_compiler_name = request.m_compiler;
            if (request.m_platform != Toolchain::host())
            {
                const Square::Vector<Square::String> host_compilers = toolchain.compilers(Toolchain::host());
                if (host_compilers.empty())
                {
                    return fail(log, "no compiler of " + Toolchain::host() + ": its tools (AssetPack, ShaderPack) make the assets");
                }
                host_compiler_name = host_compilers[0];
            }
            const Compiler& host_compiler = *toolchain.compiler(Toolchain::host(), host_compiler_name);
            const Platform& host_platform = *Toolchain::platform(Toolchain::host());
            Environment host_environment;
            if (!toolchain.environment(host_compiler, host_environment, log))
            {
                return fail(log, "no environment of " + host_compiler.m_name);
            }
            const Square::String tools = folder(root, Toolchain::host(), host_compiler.m_name, "Release");
            if (tools != build)
            {
                steps.push_back({ "configure the tools", configure(toolchain, request, host_platform, host_compiler, "Release", tools, host_environment), nullptr });
            }
            Square::Vector<Square::String> tools_arguments{ "--build", tools, "--target" };
            tools_arguments.insert(tools_arguments.end(), s_tools.begin(), s_tools.end());
            for (const Square::String& driver : drivers(Toolchain::host()))
            {
                tools_arguments.push_back(driver);
            }
            steps.push_back({ "build the tools", cmake(toolchain, tools_arguments, host_environment), nullptr });
            //the release folder made again: the game installed (a mobile: its APK, its files)
            steps.push_back({ "clean " + release, Command(), [release](Process&, const LineCallback& log) -> int
            {
                std::error_code error;
                std::filesystem::remove_all(std::filesystem::u8path(release), error);
                std::filesystem::create_directories(std::filesystem::u8path(release), error);
                if (error)
                {
                    log("cannot make " + release + ": " + text_of(error.message()));
                    return 1;
                }
                return 0;
            } });
            if (platform->m_mobile)
            {
                const Toolchain* tools_chain = &toolchain;
                steps.push_back({ "install the files of the game", Command(), [=](Process& process, const LineCallback& log) -> int
                {
                    for (const Square::String& component : components(build))
                    {
                        const Command install = cmake(*tools_chain, { "--install", build, "--component", component, "--prefix", files }, environment);
                        log("> " + Process::text(install));
                        const int code = process.run(install, log);
                        if (code != 0)
                        {
                            return code;
                        }
                    }
                    const Square::String name = lower(config);
                    const Square::String apk = join(build, "android/" + target + "/app/build/outputs/apk/" + name + "/app-" + name + ".apk");
                    std::error_code error;
                    std::filesystem::copy_file
                    (
                          std::filesystem::u8path(apk)
                        , std::filesystem::u8path(join(release, target + ".apk"))
                        , std::filesystem::copy_options::overwrite_existing
                        , error
                    );
                    if (error)
                    {
                        log("cannot copy " + apk + ": " + text_of(error.message()));
                        return 1;
                    }
                    log(apk + " -> " + join(release, target + ".apk"));
                    return 0;
                } });
            }
            else
            {
                steps.push_back({ "install the game", cmake(toolchain, { "--install", build, "--prefix", release }, environment), nullptr });
            }
            //the archives in the compression of the platform, the shaders packed
            const Square::String asset_pack = join(tools, "bin/" + Square::Environment::program_name("AssetPack"));
            const Square::String shader_pack = join(tools, "bin/" + Square::Environment::program_name("ShaderPack"));
            const Square::String archives_folder = join(root, "assets-" + request.m_platform);
            const Square::Vector<Square::String> exclude = excluded(request.m_game);
            Command asset_command;
            asset_command.m_arguments =
            {
                asset_pack,
                "-t", request.m_platform,
                "-i", archives(request.m_game, exclude),
                "-o", archives_folder,
                "-c", join(root, "assetcache-" + request.m_platform),
                "-r", join(release, "assetpack_" + request.m_platform + ".txt")
            };
            asset_command.m_directory = request.m_game;
            steps.push_back({ "clean " + archives_folder, Command(), [archives_folder](Process&, const LineCallback&) -> int
            {
                std::error_code error;
                std::filesystem::remove_all(std::filesystem::u8path(archives_folder), error);
                return 0;
            } });
            steps.push_back({ "archives", asset_command, nullptr });
            steps.push_back({ "archives in the release", Command(), [archives_folder, files, exclude](Process&, const LineCallback& log) -> int
            {
                std::error_code error;
                const std::filesystem::path assets = std::filesystem::u8path(join(files, "assets"));
                std::filesystem::create_directories(assets, error);
                for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::u8path(archives_folder), error))
                {
                    if (Square::Filesystem::get_extension(text_of(entry.path().u8string())) != ".sqz")
                    {
                        continue;
                    }
                    std::filesystem::copy_file(entry.path(), assets / entry.path().filename(), std::filesystem::copy_options::overwrite_existing, error);
                    if (error)
                    {
                        log("cannot copy " + text_of(entry.path().u8string()) + ": " + text_of(error.message()));
                        return 1;
                    }
                }
                //(the files out of the release: the tests of the game)
                for (const Square::String& name : exclude)
                {
                    std::filesystem::remove_all(assets / std::filesystem::u8path(name), error);
                }
                return 0;
            } });
            Command shader_command;
            shader_command.m_arguments =
            {
                shader_pack,
                "-t", request.m_platform,
                "-r", "resources.rs",
                "-o", join(root, "shaderpack-" + request.m_platform),
                "-p", join(files, "shaders.sqz")
            };
            shader_command.m_directory = request.m_game;
            steps.push_back({ "shaders", shader_command, nullptr });
        }
        //deploy: a mobile, the APK and its resources (the release, else the ones of the game)
        if (request.m_deploy && !platform->m_mobile && log)
        {
            log("deploy: " + request.m_platform + " has none (its release: the assets)");
        }
        //the server of adb started first, out of the steps: started by a step's adb it would keep
        //that step's output open (its handles inherited), the step never ending
        const Square::String adb = toolchain.adb();
        if (platform->m_mobile && (request.m_deploy || request.m_run))
        {
            steps.push_back({ "start adb", Command(), [adb](Process& process, const LineCallback& log) -> int
            {
                Command command;
                command.m_arguments = { adb, "start-server" };
                log("> " + Process::text(command));
                return process.run_quiet(command);
            } });
        }
        if (request.m_deploy && platform->m_mobile)
        {
            steps.push_back({ "install the APK", cmake(toolchain, { "--build", build, "--target", target + "_android_install" }, environment), nullptr });
            //(the release of a pack before: its files the game changed since copied again, the
            //packed ones told when older than their sources)
            const bool released = request.m_assets || Square::Filesystem::is_directory(files);
            if (released && !request.m_assets)
            {
                const Square::String game = request.m_game;
                steps.push_back({ "refresh the release", Command(), [=](Process&, const LineCallback& log) -> int
                {
                    return refresh_release(game, engine, files, log);
                } });
            }
            if (released)
            {
                const Square::String push = join(engine, "cmake/SquareAndroidPush.cmake");
                const Toolchain* push_chain = &toolchain;
                steps.push_back({ "push the release", Command(), [=](Process& process, const LineCallback& log) -> int
                {
                    const Square::String id = package(build, target);
                    if (id.empty())
                    {
                        log("no package in the Gradle project of " + build);
                        return 1;
                    }
                    Square::String resources;
                    for (const Square::String& name : entries(files))
                    {
                        if (!resources.empty())
                        {
                            resources += '|';
                        }
                        resources += name;
                    }
                    const Command command = cmake(*push_chain, { "-DADB=" + adb, "-DPACKAGE=" + id, "-DROOT=" + files, "-DRESOURCES=" + resources, "-P", push }, environment);
                    log("> " + Process::text(command));
                    return process.run(command, log);
                } });
            }
            else
            {
                if (log)
                {
                    log("deploy: no release of " + request.m_platform + "-" + lower(config) + " yet: the resources of the game as they are (not packed, no maps): pack assets once");
                }
                steps.push_back({ "push the resources", cmake(toolchain, { "--build", build, "--target", target + "_android_push" }, environment), nullptr });
            }
        }
        //run: on the device, on the desktop (the release, else the build from the game folder)
        if (request.m_run && platform->m_mobile)
        {
            steps.push_back({ "run", cmake(toolchain, { "--build", build, "--target", target + "_android_run" }, environment), nullptr });
        }
        if (request.m_run && !platform->m_mobile)
        {
            Command game;
            if (request.m_assets)
            {
                game.m_arguments = { join(release, "bin/" + Square::Environment::program_name(target)) };
                game.m_directory = release;
            }
            else
            {
                game.m_arguments = { join(build, "bin/" + Square::Environment::program_name(target)) };
                game.m_directory = request.m_game;
            }
            steps.push_back({ "run", game, nullptr });
        }
        return true;
    }

    void Build::print(const Square::Vector<Step>& steps, const LineCallback& log)
    {
        for (const Step& step : steps)
        {
            if (step.m_action)
            {
                log("[" + step.m_title + "]");
            }
            else
            {
                log("[" + step.m_title + "] " + Process::text(step.m_command));
            }
        }
    }

    bool Build::run(const Square::Vector<Step>& steps, Process& process, const LineCallback& log)
    {
        for (size_t i = 0; i != steps.size(); ++i)
        {
            const Step& step = steps[i];
            log("== " + step.m_title + " (" + Square::to_string(i + 1) + "/" + Square::to_string(steps.size()) + ")");
            int code = 0;
            if (process.stopped())
            {
                code = s_stopped;
            }
            else if (step.m_action)
            {
                code = step.m_action(process, log);
            }
            else
            {
                log("> " + Process::text(step.m_command));
                code = process.run(step.m_command, log);
            }
            if (code == s_stopped)
            {
                log("== stopped");
                return false;
            }
            if (code != 0)
            {
                log("== failed: " + step.m_title + " (" + Square::to_string(code) + ")");
                return false;
            }
        }
        log("== done");
        return true;
    }
}
