//
//  Toolchain.h
//  Builder
//
//  The tools of a build on this host: cmake, ninja, the compilers of each platform, the Android
//  SDK and the JDK of Gradle. Each one the one of the settings, else found (the PATH, the folders
//  where they are installed: Visual Studio by vswhere, the Android SDK by its folders).
//
#pragma once
#include <Square/Square.h>
#include "Process.h"
#include "Settings.h"

namespace Builder
{
    using Environment = Square::Map<Square::String, Square::String>;

    //a platform of a build: its name, the one of the engine (SQUARE_PLATFORM), a mobile (its
    //deploy), the host that builds it (nullptr: any)
    struct Platform
    {
        const char* m_name;
        const char* m_square;
        bool        m_mobile;
        const char* m_host;
    };

    //a compiler of a platform: its name, its platform, its programs (C, C++; empty: the ones of
    //its toolchain), Visual Studio (its environment, vcvars)
    struct Compiler
    {
        Square::String m_name;
        Square::String m_platform;
        Square::String m_c;
        Square::String m_cxx;
        bool           m_vcvars{ false };
    };

    class Toolchain
    {
    public:

        //the tools found (or given by the settings): a line each in the log
        void detect(const Settings& settings, const LineCallback& log);

        //the platforms, one by its name (nullptr: none)
        static const Square::Vector<Platform>& platforms();
        static const Platform* platform(const Square::String& name);
        //the platform of the host (windows, linux, macos)
        static Square::String host();

        //the compilers found of a platform, one by its name (nullptr: not found)
        Square::Vector<Square::String> compilers(const Square::String& platform) const;
        const Compiler* compiler(const Square::String& platform, const Square::String& name) const;
        //a compiler found (of any platform)
        bool known(const Square::String& name) const;

        //the environment of the commands of a build (Visual Studio: the one of vcvars, captured
        //once; Android: its SDK, its JDK): false, it could not be made
        bool environment(const Compiler& compiler, Environment& out, const LineCallback& log);

        //the tools
        const Square::String& cmake() const;
        const Square::String& ninja() const;
        const Square::String& android_sdk() const;
        const Square::String& java_home() const;
        Square::String adb() const;

        //the tools as a text (a line each: found or not)
        Square::String report() const;

    private:

        Square::String          m_cmake;
        Square::String          m_ninja;
        Square::String          m_visual_studio;
        Square::String          m_vcvars;
        Square::String          m_android_sdk;
        Square::String          m_java_home;
        Square::Vector<Compiler> m_compilers;
        Environment             m_vcvars_environment; //(empty: not captured yet)
    };
}
