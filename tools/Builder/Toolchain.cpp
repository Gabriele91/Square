//
//  Toolchain.cpp
//  Builder
//
//  See Toolchain.h.
//
#include "Toolchain.h"
#include <algorithm>
#include <cstdlib>
#include <filesystem>

namespace Builder
{
    namespace AuxToolchain
    {
        //the platforms of a build (the desktop ones built by their own host)
        static const Square::Vector<Platform> s_platforms
        {
            { "windows", "desktop", false, "windows" },
            { "linux",   "desktop", false, "linux"   },
            { "macos",   "desktop", false, "macos"   },
            { "android", "android", true,  nullptr   },
        };

        //a text of the standard library (std::filesystem) in one of the engine
        static Square::String text_of(const std::string& text)
        {
            return Square::String(text.c_str(), text.size());
        }

        //a path with '/' (the one of cmake)
        static Square::String slashes(Square::String path)
        {
            std::replace(path.begin(), path.end(), '\\', '/');
            return path;
        }

        //a variable of the environment (empty: none), with '/'
        static Square::String variable(const char* name)
        {
            return slashes(Square::Environment::variable(name));
        }

        //a program of the PATH (empty: none), with '/'
        static Square::String in_path(const Square::String& name)
        {
            return slashes(Square::Environment::find_program(name));
        }

        //the first file of a list that is there (empty: none)
        static Square::String first_file(const Square::Vector<Square::String>& files)
        {
            for (const Square::String& file : files)
            {
                if (!file.empty() && Square::Filesystem::is_file(file))
                {
                    return slashes(file);
                }
            }
            return Square::String();
        }

        //the folders in a folder whose names start so, the newest version first (the names
        //sorted down: 3.31.6 before 3.22.1)
        static Square::Vector<Square::String> folders(const Square::String& parent, const Square::String& prefix)
        {
            Square::Vector<Square::String> out;
            std::error_code error;
            if (parent.empty() || !std::filesystem::is_directory(std::filesystem::u8path(parent), error))
            {
                return out;
            }
            for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::u8path(parent), error))
            {
                const Square::String name = text_of(entry.path().filename().u8string());
                if (entry.is_directory() && name.compare(0, prefix.size(), prefix) == 0)
                {
                    out.push_back(slashes(text_of(entry.path().u8string())));
                }
            }
            std::sort(out.begin(), out.end(), std::greater<Square::String>());
            return out;
        }

        //the files of a name in the folders of a folder (<parent>/<prefix>*/<file>), newest first
        static Square::Vector<Square::String> in_folders(const Square::String& parent, const Square::String& prefix, const Square::String& file)
        {
            Square::Vector<Square::String> out;
            for (const Square::String& folder : folders(parent, prefix))
            {
                out.push_back(Square::Filesystem::join(folder, file));
            }
            return out;
        }

        //a place where a tool is installed by default: a folder (under a variable of the user:
        //HOME, LOCALAPPDATA; nullptr: absolute), else the folders in it whose names start so (a
        //version: jdk-17.0.12), their sub folder (suffix)
        struct Place
        {
            const char* m_variable;
            const char* m_path;
            const char* m_prefix;
            const char* m_suffix;
        };

    #if defined(_WIN32)
        static const Place s_sdk_places[] =
        {
            { "LOCALAPPDATA", "Android/Sdk",                             nullptr, nullptr },
            { "APPDATA",      "Android/Sdk",                             nullptr, nullptr },
            { "USERPROFILE",  "Android/Sdk",                             nullptr, nullptr },
            { nullptr,        "C:/Android/Sdk",                          nullptr, nullptr },
            { nullptr,        "C:/Program Files (x86)/Android/android-sdk", nullptr, nullptr },
        };
        static const Place s_jdk_places[] =
        {
            { nullptr,        "C:/Program Files/Java",                   "jdk-17",  nullptr },
            { nullptr,        "C:/Program Files/Eclipse Adoptium",       "jdk-17",  nullptr },
            { nullptr,        "C:/Program Files/Microsoft",              "jdk-17",  nullptr },
            { nullptr,        "C:/Program Files/Zulu",                   "zulu-17", nullptr },
            { nullptr,        "C:/Program Files/Android/Android Studio/jbr", nullptr, nullptr },
            { "LOCALAPPDATA", "Programs/Android Studio/jbr",             nullptr, nullptr },
        };
    #elif defined(__APPLE__)
        static const Place s_sdk_places[] =
        {
            { "HOME",  "Library/Android/sdk",                            nullptr, nullptr },
            { nullptr, "/opt/homebrew/share/android-commandlinetools",   nullptr, nullptr },
            { nullptr, "/usr/local/share/android-commandlinetools",      nullptr, nullptr },
        };
        static const Place s_jdk_places[] =
        {
            { nullptr, "/Library/Java/JavaVirtualMachines", "temurin-17", "Contents/Home" },
            { nullptr, "/Library/Java/JavaVirtualMachines", "jdk-17",     "Contents/Home" },
            { nullptr, "/Library/Java/JavaVirtualMachines", "zulu-17",    "Contents/Home" },
            { nullptr, "/opt/homebrew/opt/openjdk@17/libexec/openjdk.jdk/Contents/Home", nullptr, nullptr },
            { nullptr, "/usr/local/opt/openjdk@17/libexec/openjdk.jdk/Contents/Home",    nullptr, nullptr },
            { nullptr, "/Applications/Android Studio.app/Contents/jbr/Contents/Home",    nullptr, nullptr },
        };
    #else
        static const Place s_sdk_places[] =
        {
            { "HOME",  "Android/Sdk",          nullptr, nullptr },
            { "HOME",  "Android/sdk",          nullptr, nullptr },
            { "HOME",  "android-sdk",          nullptr, nullptr },
            { nullptr, "/opt/android-sdk",     nullptr, nullptr },
            { nullptr, "/usr/lib/android-sdk", nullptr, nullptr },
        };
        static const Place s_jdk_places[] =
        {
            { nullptr, "/usr/lib/jvm",                     "java-17",    nullptr },
            { nullptr, "/usr/lib/jvm",                     "temurin-17", nullptr },
            { nullptr, "/usr/lib/jvm",                     "jdk-17",     nullptr },
            { "HOME",  "android-studio/jbr",               nullptr,      nullptr },
            { nullptr, "/opt/android-studio/jbr",          nullptr,      nullptr },
            { nullptr, "/snap/android-studio/current/jbr", nullptr,      nullptr },
        };
    #endif

        //the folders of the places of a tool, in their order
        template <size_t N>
        static Square::Vector<Square::String> places(const Place (&table)[N])
        {
            Square::Vector<Square::String> out;
            for (const Place& place : table)
            {
                Square::String base = place.m_path;
                if (place.m_variable)
                {
                    const Square::String root = variable(place.m_variable);
                    if (root.empty())
                    {
                        continue;
                    }
                    base = Square::Filesystem::join(root, place.m_path);
                }
                if (!place.m_prefix)
                {
                    out.push_back(base);
                    continue;
                }
                for (const Square::String& folder : folders(base, place.m_prefix))
                {
                    if (place.m_suffix)
                    {
                        out.push_back(Square::Filesystem::join(folder, place.m_suffix));
                    }
                    else
                    {
                        out.push_back(folder);
                    }
                }
            }
            return out;
        }

        //the first line of the output of a command (empty: none, it failed)
        static Square::String first_line(const Command& command)
        {
            Process process;
            Square::String out;
            const int code = process.run(command, [&](const Square::String& line)
            {
                if (out.empty())
                {
                    out = line;
                }
            });
            if (code != 0)
            {
                return Square::String();
            }
            return out;
        }

        //the sibling of a program (gcc: g++, clang: clang++), else its name
        static Square::String sibling(const Square::String& program, const Square::String& from, const Square::String& to)
        {
            const size_t at = program.rfind(from);
            if (at == Square::String::npos)
            {
                return to;
            }
            const Square::String out = program.substr(0, at) + to + program.substr(at + from.size());
            if (Square::Filesystem::is_file(out))
            {
                return out;
            }
            return to;
        }

        //a line of the report: a tool and where it is
        static Square::String line(const char* name, const Square::String& path)
        {
            Square::String out = Square::String(name) + ": ";
            if (path.empty())
            {
                return out + "not found\n";
            }
            return out + path + "\n";
        }
    }

    const Square::Vector<Platform>& Toolchain::platforms()
    {
        return AuxToolchain::s_platforms;
    }

    const Platform* Toolchain::platform(const Square::String& name)
    {
        for (const Platform& platform : AuxToolchain::s_platforms)
        {
            if (name == platform.m_name)
            {
                return &platform;
            }
        }
        return nullptr;
    }

    Square::String Toolchain::host()
    {
    #if defined(_WIN32)
        return "windows";
    #elif defined(__APPLE__)
        return "macos";
    #else
        return "linux";
    #endif
    }

    void Toolchain::detect(const Settings& settings, const LineCallback& log)
    {
        using namespace AuxToolchain;
        using Square::Filesystem::join;
        m_compilers.clear();
        m_vcvars_environment.clear();
        m_visual_studio.clear();
        //Visual Studio (its C++ tools): vswhere
    #if defined(_WIN32)
        const Square::String vswhere = join(variable("ProgramFiles(x86)"), "Microsoft Visual Studio/Installer/vswhere.exe");
        if (Square::Filesystem::is_file(vswhere))
        {
            Command query;
            query.m_arguments =
            {
                vswhere, "-latest", "-products", "*",
                "-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
                "-property", "installationPath"
            };
            m_visual_studio = slashes(first_line(query));
        }
        m_vcvars = first_file({ settings.m_vcvars, join(m_visual_studio, "VC/Auxiliary/Build/vcvars64.bat") });
    #endif
        //the Android SDK (its platform tools), the JDK of Gradle: the settings, the variables, then
        //where they are installed by default (Android Studio, the packages of the system)
        Square::Vector<Square::String> sdks{ settings.m_android_sdk, variable("ANDROID_HOME"), variable("ANDROID_SDK_ROOT") };
        for (const Square::String& place : places(s_sdk_places))
        {
            sdks.push_back(place);
        }
        m_android_sdk.clear();
        for (const Square::String& sdk : sdks)
        {
            if (!sdk.empty() && Square::Filesystem::is_directory(join(sdk, "platform-tools")))
            {
                m_android_sdk = slashes(sdk);
                break;
            }
        }
        const Square::String java = Square::Environment::program_name("bin/java");
        Square::Vector<Square::String> javas{ settings.m_java_home, variable("JAVA_HOME") };
        for (const Square::String& place : places(s_jdk_places))
        {
            javas.push_back(place);
        }
        m_java_home.clear();
        for (const Square::String& home : javas)
        {
            if (!home.empty() && Square::Filesystem::is_file(join(home, java)))
            {
                m_java_home = slashes(home);
                break;
            }
        }
        //cmake, ninja: the settings, the PATH, the ones of Visual Studio, of the Android SDK
        const Square::String vs_cmake = join(m_visual_studio, "Common7/IDE/CommonExtensions/Microsoft/CMake");
        Square::Vector<Square::String> cmakes{ settings.m_cmake, in_path("cmake") };
        Square::Vector<Square::String> ninjas{ settings.m_ninja, in_path("ninja") };
    #if defined(_WIN32)
        cmakes.push_back("C:/Program Files/CMake/bin/cmake.exe");
        cmakes.push_back(join(vs_cmake, "CMake/bin/cmake.exe"));
        ninjas.push_back(join(vs_cmake, "Ninja/ninja.exe"));
    #elif defined(__APPLE__)
        cmakes.push_back("/Applications/CMake.app/Contents/bin/cmake");
        cmakes.push_back("/opt/homebrew/bin/cmake");
        ninjas.push_back("/opt/homebrew/bin/ninja");
    #endif
        for (const Square::String& file : in_folders(join(m_android_sdk, "cmake"), "", Square::Environment::program_name("bin/cmake")))
        {
            cmakes.push_back(file);
        }
        for (const Square::String& file : in_folders(join(m_android_sdk, "cmake"), "", Square::Environment::program_name("bin/ninja")))
        {
            ninjas.push_back(file);
        }
        m_cmake = first_file(cmakes);
        m_ninja = first_file(ninjas);
        //the compilers of each platform
    #if defined(_WIN32)
        if (!m_vcvars.empty())
        {
            m_compilers.push_back({ "msvc", "windows", "cl", "cl", true });
            const Square::String clang_cl = first_file
            ({
                settings.m_clang_cl,
                join(m_visual_studio, "VC/Tools/Llvm/x64/bin/clang-cl.exe"),
                "C:/Program Files/LLVM/bin/clang-cl.exe",
                in_path("clang-cl")
            });
            if (!clang_cl.empty())
            {
                m_compilers.push_back({ "clang-cl", "windows", clang_cl, clang_cl, true });
            }
        }
    #elif defined(__APPLE__)
        const Square::String clang = first_file({ settings.m_clang, "/usr/bin/clang", in_path("clang") });
        if (!clang.empty())
        {
            m_compilers.push_back({ "clang", "macos", clang, sibling(clang, "clang", "clang++"), false });
        }
    #else
        const Square::String gcc = first_file({ settings.m_gcc, in_path("gcc") });
        if (!gcc.empty())
        {
            m_compilers.push_back({ "gcc", "linux", gcc, sibling(gcc, "gcc", "g++"), false });
        }
        const Square::String clang = first_file({ settings.m_clang, in_path("clang") });
        if (!clang.empty())
        {
            m_compilers.push_back({ "clang", "linux", clang, sibling(clang, "clang", "clang++"), false });
        }
    #endif
        //Android: the NDK of the SDK (its toolchain chosen by the engine, SquarePlatform.cmake)
        for (const Square::String& ndk : folders(join(m_android_sdk, "ndk"), ""))
        {
            if (Square::Filesystem::is_file(join(ndk, "build/cmake/android.toolchain.cmake")))
            {
                m_compilers.push_back({ "ndk", "android", "", "", false });
                break;
            }
        }
        if (log)
        {
            Square::String text = report();
            size_t start = 0;
            for (size_t end = text.find('\n'); end != Square::String::npos; end = text.find('\n', start))
            {
                log(text.substr(start, end - start));
                start = end + 1;
            }
        }
    }

    Square::Vector<Square::String> Toolchain::compilers(const Square::String& platform) const
    {
        Square::Vector<Square::String> out;
        for (const Compiler& compiler : m_compilers)
        {
            if (compiler.m_platform == platform)
            {
                out.push_back(compiler.m_name);
            }
        }
        return out;
    }

    bool Toolchain::known(const Square::String& name) const
    {
        for (const Compiler& compiler : m_compilers)
        {
            if (compiler.m_name == name)
            {
                return true;
            }
        }
        return false;
    }

    const Compiler* Toolchain::compiler(const Square::String& platform, const Square::String& name) const
    {
        for (const Compiler& compiler : m_compilers)
        {
            if (compiler.m_platform == platform && compiler.m_name == name)
            {
                return &compiler;
            }
        }
        return nullptr;
    }

    bool Toolchain::environment(const Compiler& compiler, Environment& out, const LineCallback& log)
    {
        using Square::Filesystem::join;
        out = Square::Environment::all();
        if (compiler.m_vcvars)
        {
            //the environment of vcvars, once: a script that calls it, then writes all of it
            if (m_vcvars_environment.empty())
            {
                if (log)
                {
                    log("vcvars: " + m_vcvars);
                }
                std::error_code error;
                const Square::String script = AuxToolchain::slashes(AuxToolchain::text_of(std::filesystem::temp_directory_path(error).u8string()) + "/square_builder_vcvars.bat");
                Square::Filesystem::text_file_write_all(script, "@call \"" + m_vcvars + "\" >nul\r\n@set\r\n");
                Command command;
                command.m_arguments = { "cmd", "/d", "/c", script };
                Process process;
                Environment captured;
                process.run(command, [&](const Square::String& line)
                {
                    Square::String name;
                    Square::String value;
                    if (Square::Environment::parse(line, name, value))
                    {
                        captured[name] = value;
                    }
                });
                std::filesystem::remove(std::filesystem::u8path(script), error);
                if (captured.find("VCToolsInstallDir") == captured.end())
                {
                    if (log)
                    {
                        log("vcvars: no environment of Visual Studio from " + m_vcvars);
                    }
                    return false;
                }
                m_vcvars_environment = captured;
            }
            out = m_vcvars_environment;
        }
        if (compiler.m_platform == "android")
        {
            if (m_android_sdk.empty() || m_java_home.empty())
            {
                if (log)
                {
                    log("android: no Android SDK or no JDK 17 (the settings: android_sdk, java_home)");
                }
                return false;
            }
            out["ANDROID_HOME"] = m_android_sdk;
            out["JAVA_HOME"] = m_java_home;
        }
        return true;
    }

    const Square::String& Toolchain::cmake() const
    {
        return m_cmake;
    }

    const Square::String& Toolchain::ninja() const
    {
        return m_ninja;
    }

    const Square::String& Toolchain::android_sdk() const
    {
        return m_android_sdk;
    }

    const Square::String& Toolchain::java_home() const
    {
        return m_java_home;
    }

    Square::String Toolchain::adb() const
    {
        if (m_android_sdk.empty())
        {
            return Square::String();
        }
        return AuxToolchain::slashes(Square::Filesystem::join(m_android_sdk, Square::Environment::program_name("platform-tools/adb")));
    }

    Square::String Toolchain::report() const
    {
        using AuxToolchain::line;
        Square::String out;
        out += line("cmake", m_cmake);
        out += line("ninja", m_ninja);
    #if defined(_WIN32)
        out += line("visual studio", m_visual_studio);
        out += line("vcvars", m_vcvars);
    #endif
        out += line("android sdk", m_android_sdk);
        out += line("java home", m_java_home);
        for (const Platform& platform : AuxToolchain::s_platforms)
        {
            const bool here = !platform.m_host || host() == platform.m_host;
            if (!here)
            {
                continue;
            }
            Square::String names;
            for (const Square::String& name : compilers(platform.m_name))
            {
                if (!names.empty())
                {
                    names += ", ";
                }
                names += name;
            }
            out += line(platform.m_name, names);
        }
        return out;
    }
}
