//
//  Build.h
//  Builder
//
//  A build of a game: its request (the game, its target, the platform, the configuration, the
//  compiler; its assets, its deploy, its run) made into its steps, then run one after the other,
//  stopped at the first that fails:
//
//      configure   cmake -G Ninja (<build root>/<platform>-<compiler>-<config>)
//      build       the target (Android: its APK, <target>_apk; the desktop: with its drivers)
//      assets      the release of the platform (as engine/cmake/SquareRelease.cmake): the tools
//                  of the host (AssetPack, ShaderPack), the game installed, its archives in the
//                  compression of its GPUs, its shaders packed (<build root>/release/<platform>-<config>)
//      deploy      a mobile: the APK installed, its resources pushed (the ones of the release,
//                  else the ones of the game: <target>_android_push)
//      run         the game started (on the device, on the desktop)
//
#pragma once
#include <Square/Square.h>
#include "Process.h"
#include "Settings.h"
#include "Toolchain.h"

namespace Builder
{
    //what is built
    struct Request
    {
        Square::String m_game;
        Square::String m_target;
        Square::String m_engine;
        Square::String m_build_root;
        Square::String m_platform;
        Square::String m_config;
        Square::String m_compiler;
        bool           m_assets{ false };
        bool           m_deploy{ false };
        bool           m_run{ false };

        //the one of the settings, the settings of it
        static Request from(const Settings& settings);
        void to(Settings& settings) const;
    };

    //an action of the tool (a step that is not one command): its code (0: done)
    using Action = std::function<int(Process& process, const LineCallback& log)>;

    //a step: its title, its command (else its action)
    struct Step
    {
        Square::String m_title;
        Command        m_command;
        Action         m_action;
    };

    class Build
    {
    public:

        //the configurations (Debug, Release, Retail)
        static const Square::Vector<Square::String>& configs();

        //the steps of a request, its values not given filled (the platform of the host, its first
        //compiler...): false, it cannot be built (why: in the log)
        static bool plan(Request& request, Toolchain& toolchain, Square::Vector<Step>& steps, const LineCallback& log);
        //the steps as a text (a line each)
        static void print(const Square::Vector<Step>& steps, const LineCallback& log);
        //the steps run (false: one failed, or stopped)
        static bool run(const Square::Vector<Step>& steps, Process& process, const LineCallback& log);
    };
}
