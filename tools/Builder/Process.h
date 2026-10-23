//
//  Process.h
//  Builder
//
//  A command run to its end: its output by lines (its standard output and error together), its
//  code; its folder, its environment (none: the one of the tool). Stopped from another thread
//  with all the processes it started (a job on Windows, a group elsewhere: cmake, ninja, the
//  compilers under them).
//
#pragma once
#include <atomic>
#include <functional>
#include <mutex>
#include <Square/Square.h>

namespace Builder
{
    //a command: its program and its arguments, its folder (empty: the one of the tool), its
    //environment (empty: the one of the tool)
    struct Command
    {
        Square::Vector<Square::String>                 m_arguments;
        Square::String                                 m_directory;
        Square::Map<Square::String, Square::String>    m_environment;
    };

    //a line of the output of a process (no end of line)
    using LineCallback = std::function<void(const Square::String& line)>;

    //the codes of a process not run to its end
    static constexpr int s_not_started = -1;
    static constexpr int s_stopped = -2;

    class Process
    {
    public:

        Process();
        ~Process();

        //a command run to its end, each line of its output given: its code (s_not_started: no
        //program; s_stopped: stopped)
        int run(const Command& command, const LineCallback& line);
        //a command run to its end, nothing of the tool given to it (no pipe, no handle, no output
        //read): a server it starts (adb's) holds nothing of the tool, never the output of a step
        int run_quiet(const Command& command);
        //the one running stopped (from another thread), the next ones not started
        void stop();
        //stopped (until reset)
        bool stopped() const;
        void reset();

        //a command as it is written (its arguments quoted where they need it): a log, a plan
        static Square::String text(const Command& command);

    private:

        std::atomic<bool> m_stop{ false };
        std::mutex        m_mutex;
        void*             m_running{ nullptr }; //Windows: its job; else: the id of its group
    };
}
