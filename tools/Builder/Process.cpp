//
//  Process.cpp
//  Builder
//
//  See Process.h.
//
#include "Process.h"
#include <cstring>
#if defined(_WIN32)
#define NOMINMAX
#include <Windows.h>
#else
#include <csignal>
#include <fcntl.h>
#include <spawn.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
extern char** environ;
#endif

namespace Builder
{
    namespace AuxProcess
    {
        //the lines of a buffer read (the rest: the start of the next one); a \r before a \n out
        static void lines(Square::String& pending, const char* data, size_t size, const LineCallback& line)
        {
            pending.append(data, size);
            size_t start = 0;
            for (size_t end = pending.find('\n'); end != Square::String::npos; end = pending.find('\n', start))
            {
                size_t length = end - start;
                if (length && pending[start + length - 1] == '\r')
                {
                    --length;
                }
                if (line)
                {
                    line(pending.substr(start, length));
                }
                start = end + 1;
            }
            pending.erase(0, start);
        }

        //the last line without its end
        static void flush(Square::String& pending, const LineCallback& line)
        {
            if (!pending.empty() && line)
            {
                line(pending);
            }
            pending.clear();
        }

        //an argument as written: quoted when it has a space, a quote or nothing
        static Square::String quoted(const Square::String& argument)
        {
            const bool plain = !argument.empty() && argument.find_first_of(" \t\"") == Square::String::npos;
            if (plain)
            {
                return argument;
            }
            //(the rules of CommandLineToArgvW: the backslashes before a quote doubled)
            Square::String out = "\"";
            size_t backslashes = 0;
            for (char c : argument)
            {
                if (c == '\\')
                {
                    ++backslashes;
                    continue;
                }
                if (c == '"')
                {
                    out.append(backslashes * 2 + 1, '\\');
                }
                else
                {
                    out.append(backslashes, '\\');
                }
                backslashes = 0;
                out += c;
            }
            out.append(backslashes * 2, '\\');
            out += '"';
            return out;
        }

#if defined(_WIN32)
        static std::wstring wide(const Square::String& text)
        {
            if (text.empty())
            {
                return std::wstring();
            }
            const int size = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), int(text.size()), nullptr, 0);
            std::wstring out(size_t(size), L'\0');
            MultiByteToWideChar(CP_UTF8, 0, text.c_str(), int(text.size()), &out[0], size);
            return out;
        }

        //an environment block: NAME=VALUE\0 ... \0\0 (sorted by name, as Windows wants it)
        static std::wstring environment_block(const Square::Map<Square::String, Square::String>& environment)
        {
            std::wstring out;
            for (const auto& [name, value] : environment)
            {
                out += wide(name + "=" + value);
                out += L'\0';
            }
            out += L'\0';
            return out;
        }
#endif
    }

    Process::Process()
    {
    }

    Process::~Process()
    {
        stop();
    }

    bool Process::stopped() const
    {
        return m_stop;
    }

    void Process::reset()
    {
        m_stop = false;
    }

    Square::String Process::text(const Command& command)
    {
        Square::String out;
        for (const Square::String& argument : command.m_arguments)
        {
            if (!out.empty())
            {
                out += ' ';
            }
            out += AuxProcess::quoted(argument);
        }
        return out;
    }

#if defined(_WIN32)
    void Process::stop()
    {
        m_stop = true;
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_running)
        {
            TerminateJobObject(HANDLE(m_running), 1);
        }
    }

    int Process::run(const Command& command, const LineCallback& line)
    {
        if (m_stop)
        {
            return s_stopped;
        }
        if (command.m_arguments.empty())
        {
            return s_not_started;
        }
        //its output: a pipe (its end in the process inherited)
        SECURITY_ATTRIBUTES security{ sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };
        HANDLE read = nullptr;
        HANDLE write = nullptr;
        if (!CreatePipe(&read, &write, &security, 0))
        {
            return s_not_started;
        }
        SetHandleInformation(read, HANDLE_FLAG_INHERIT, 0);
        //its input: none
        HANDLE input = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &security, OPEN_EXISTING, 0, nullptr);
        STARTUPINFOW startup{};
        startup.cb = sizeof(startup);
        startup.dwFlags = STARTF_USESTDHANDLES;
        startup.hStdInput = input;
        startup.hStdOutput = write;
        startup.hStdError = write;
        std::wstring command_line = AuxProcess::wide(text(command));
        std::wstring directory = AuxProcess::wide(command.m_directory);
        std::wstring environment;
        if (!command.m_environment.empty())
        {
            environment = AuxProcess::environment_block(command.m_environment);
        }
        //its job: stopped with all it starts
        HANDLE job = CreateJobObjectW(nullptr, nullptr);
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
        limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        SetInformationJobObject(job, JobObjectExtendedLimitInformation, &limits, sizeof(limits));
        //(none: the ones of the tool)
        void* environment_block = nullptr;
        if (!environment.empty())
        {
            environment_block = &environment[0];
        }
        const wchar_t* folder = nullptr;
        if (!directory.empty())
        {
            folder = directory.c_str();
        }
        PROCESS_INFORMATION process{};
        const DWORD flags = CREATE_UNICODE_ENVIRONMENT | CREATE_SUSPENDED | CREATE_NO_WINDOW;
        const BOOL started = CreateProcessW
        (
              nullptr
            , &command_line[0]
            , nullptr
            , nullptr
            , TRUE
            , flags
            , environment_block
            , folder
            , &startup
            , &process
        );
        CloseHandle(write);
        if (input != INVALID_HANDLE_VALUE)
        {
            CloseHandle(input);
        }
        if (!started)
        {
            CloseHandle(read);
            CloseHandle(job);
            if (line)
            {
                line("cannot start: " + text(command));
            }
            return s_not_started;
        }
        AssignProcessToJobObject(job, process.hProcess);
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_running = job;
        }
        ResumeThread(process.hThread);
        CloseHandle(process.hThread);
        //its output, a line at a time, to its end
        char buffer[4096];
        DWORD size = 0;
        Square::String pending;
        while (ReadFile(read, buffer, sizeof(buffer), &size, nullptr) && size)
        {
            AuxProcess::lines(pending, buffer, size_t(size), line);
        }
        AuxProcess::flush(pending, line);
        WaitForSingleObject(process.hProcess, INFINITE);
        DWORD code = 0;
        GetExitCodeProcess(process.hProcess, &code);
        CloseHandle(process.hProcess);
        CloseHandle(read);
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_running = nullptr;
        }
        CloseHandle(job);
        if (m_stop)
        {
            return s_stopped;
        }
        return int(code);
    }
#else
    void Process::stop()
    {
        m_stop = true;
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_running)
        {
            //(its group: the process and all it started)
            kill(-pid_t(intptr_t(m_running)), SIGTERM);
        }
    }

    int Process::run(const Command& command, const LineCallback& line)
    {
        if (m_stop)
        {
            return s_stopped;
        }
        if (command.m_arguments.empty())
        {
            return s_not_started;
        }
        int pipes[2];
        if (pipe(pipes) != 0)
        {
            return s_not_started;
        }
        //its arguments, its environment (NAME=VALUE)
        Square::Vector<char*> arguments;
        for (const Square::String& argument : command.m_arguments)
        {
            arguments.push_back(const_cast<char*>(argument.c_str()));
        }
        arguments.push_back(nullptr);
        Square::Vector<Square::String> variables;
        for (const auto& [name, value] : command.m_environment)
        {
            variables.push_back(name + "=" + value);
        }
        Square::Vector<char*> environment;
        for (const Square::String& variable : variables)
        {
            environment.push_back(const_cast<char*>(variable.c_str()));
        }
        environment.push_back(nullptr);
        const pid_t child = fork();
        if (child < 0)
        {
            close(pipes[0]);
            close(pipes[1]);
            return s_not_started;
        }
        if (child == 0)
        {
            //the child: a group of its own, its output into the pipe, its folder, the program
            setpgid(0, 0);
            dup2(pipes[1], STDOUT_FILENO);
            dup2(pipes[1], STDERR_FILENO);
            close(pipes[0]);
            close(pipes[1]);
            const int input = open("/dev/null", O_RDONLY);
            if (input >= 0)
            {
                dup2(input, STDIN_FILENO);
            }
            if (!command.m_directory.empty() && chdir(command.m_directory.c_str()) != 0)
            {
                _exit(127);
            }
            //(none: the one of the tool)
            if (!command.m_environment.empty())
            {
                environ = environment.data();
            }
            execvp(arguments[0], arguments.data());
            _exit(127);
        }
        close(pipes[1]);
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_running = (void*)intptr_t(child);
        }
        char buffer[4096];
        Square::String pending;
        for (ssize_t size = read(pipes[0], buffer, sizeof(buffer)); size > 0; size = read(pipes[0], buffer, sizeof(buffer)))
        {
            AuxProcess::lines(pending, buffer, size_t(size), line);
        }
        AuxProcess::flush(pending, line);
        close(pipes[0]);
        int status = 0;
        waitpid(child, &status, 0);
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_running = nullptr;
        }
        if (m_stop)
        {
            return s_stopped;
        }
        if (WIFEXITED(status))
        {
            return WEXITSTATUS(status);
        }
        return s_not_started;
    }
#endif
}
