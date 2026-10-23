//
//  Settings.h
//  Builder
//
//  What the builder keeps between two runs (<app data>/square/builder.json): the tools it was told
//  (empty: found by itself), the last build asked (the CLI starts from it, the UI shows it).
//
#pragma once
#include <Square/Square.h>

namespace Builder
{
    struct Settings
    {
        //the tools (empty: found, Toolchain::detect)
        Square::String m_cmake;
        Square::String m_ninja;
        Square::String m_vcvars;      //Windows: vcvars64.bat of Visual Studio (MSVC, clang-cl)
        Square::String m_clang_cl;    //Windows: clang-cl.exe
        Square::String m_clang;       //Linux, macOS: clang (its clang++ next to it)
        Square::String m_gcc;         //Linux: gcc (its g++ next to it)
        Square::String m_android_sdk;
        Square::String m_java_home;   //the JDK of Gradle (the APK)
        //the last build asked
        Square::String m_game;        //the folder of the game (its CMakeLists.txt)
        Square::String m_target{ "Rush" };
        Square::String m_engine;      //the folder of the engine (empty: <game>/engine, else the one of the tool)
        Square::String m_build_root;  //the folder of the builds (empty: <game>/builds)
        Square::String m_platform;    //windows, linux, macos, android (empty: the host)
        Square::String m_config{ "Release" };
        Square::String m_compiler;    //empty: the first one found for the platform
        bool           m_assets{ false };
        bool           m_deploy{ false };
        bool           m_run{ false };

        //its file
        static Square::String path();
        //read (false: none, its defaults), written
        bool load();
        bool save() const;
    };
}
