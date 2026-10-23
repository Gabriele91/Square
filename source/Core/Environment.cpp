//
//  Environment.cpp
//  Square
//
//  See Environment.h.
//
#include "Square/Core/Environment.h"
#include "Square/Core/Filesystem.h"
#include "Square/Core/StringUtilities.h"
#include <cstdlib>
#include <string>

#ifdef _WIN32
    #include <windows.h>
#else
    extern char** environ;
#endif

namespace Square
{
namespace Environment
{
    namespace AuxEnvironment
    {
    #ifdef _WIN32
        //UTF-8 to the text of Windows, back
        static std::wstring wide(const String& text)
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

        static String narrow(const wchar_t* text, size_t size)
        {
            if (!size)
            {
                return String();
            }
            const int bytes = WideCharToMultiByte(CP_UTF8, 0, text, int(size), nullptr, 0, nullptr, nullptr);
            String out(size_t(bytes), '\0');
            WideCharToMultiByte(CP_UTF8, 0, text, int(size), &out[0], bytes, nullptr, nullptr);
            return out;
        }
    #endif

        //the items of a text between its separators (the empty ones out)
        static Vector<String> split(const String& text, char separator)
        {
            Vector<String> out;
            size_t start = 0;
            while (start <= text.size())
            {
                size_t end = text.find(separator, start);
                if (end == String::npos)
                {
                    end = text.size();
                }
                if (end > start)
                {
                    out.push_back(text.substr(start, end - start));
                }
                start = end + 1;
            }
            return out;
        }

        //a name of a variable, a value of it, the ones of the process: in a map
        static void add(Map<String, String>& out, const String& entry)
        {
            //(Windows: the hidden ones of cmd, "=C:=C:\...", their name from the second char)
            const size_t equal = entry.find('=', 1);
            if (equal != String::npos)
            {
                out[entry.substr(0, equal)] = entry.substr(equal + 1);
            }
        }
    }

#ifdef _WIN32
    String variable(const String& name)
    {
        const std::wstring key = AuxEnvironment::wide(name);
        const DWORD size = GetEnvironmentVariableW(key.c_str(), nullptr, 0);
        if (!size)
        {
            return String();
        }
        std::wstring value(size_t(size), L'\0');
        const DWORD written = GetEnvironmentVariableW(key.c_str(), &value[0], size);
        return AuxEnvironment::narrow(value.c_str(), size_t(written));
    }

    bool has(const String& name)
    {
        return GetEnvironmentVariableW(AuxEnvironment::wide(name).c_str(), nullptr, 0) != 0;
    }

    bool set(const String& name, const String& value)
    {
        //(the C runtime too: std::getenv reads its own copy)
        return _wputenv_s(AuxEnvironment::wide(name).c_str(), AuxEnvironment::wide(value).c_str()) == 0;
    }

    bool remove(const String& name)
    {
        return _wputenv_s(AuxEnvironment::wide(name).c_str(), L"") == 0;
    }

    Map<String, String> all()
    {
        Map<String, String> out;
        wchar_t* block = GetEnvironmentStringsW();
        if (!block)
        {
            return out;
        }
        for (const wchar_t* entry = block; *entry; entry += wcslen(entry) + 1)
        {
            AuxEnvironment::add(out, AuxEnvironment::narrow(entry, wcslen(entry)));
        }
        FreeEnvironmentStringsW(block);
        return out;
    }

    char path_separator()
    {
        return ';';
    }
#else
    String variable(const String& name)
    {
        const char* value = std::getenv(name.c_str());
        if (!value)
        {
            return String();
        }
        return value;
    }

    bool has(const String& name)
    {
        return std::getenv(name.c_str()) != nullptr;
    }

    bool set(const String& name, const String& value)
    {
        return setenv(name.c_str(), value.c_str(), 1) == 0;
    }

    bool remove(const String& name)
    {
        return unsetenv(name.c_str()) == 0;
    }

    Map<String, String> all()
    {
        Map<String, String> out;
        for (char** entry = environ; entry && *entry; ++entry)
        {
            AuxEnvironment::add(out, *entry);
        }
        return out;
    }

    char path_separator()
    {
        return ':';
    }
#endif

    String variable(const String& name, const String& fallback)
    {
        if (!has(name))
        {
            return fallback;
        }
        return variable(name);
    }

    bool is(const String& name, const String& value)
    {
        return has(name) && case_insensitive_equal(variable(name), value);
    }

    long integer(const String& name, long fallback)
    {
        const String text = variable(name);
        if (text.empty())
        {
            return fallback;
        }
        char* end = nullptr;
        const long value = std::strtol(text.c_str(), &end, 10);
        if (end == text.c_str())
        {
            return fallback;
        }
        return value;
    }

    double real(const String& name, double fallback)
    {
        const String text = variable(name);
        if (text.empty())
        {
            return fallback;
        }
        char* end = nullptr;
        const double value = std::strtod(text.c_str(), &end);
        if (end == text.c_str())
        {
            return fallback;
        }
        return value;
    }

    Vector<String> list(const String& name, char separator)
    {
        return AuxEnvironment::split(variable(name), separator);
    }

    bool parse(String line, String& name, String& value)
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        if (line.empty() || line[0] == '#')
        {
            return false;
        }
        const size_t equal = line.find('=');
        if (equal == String::npos || equal == 0)
        {
            return false;
        }
        name = line.substr(0, equal);
        value = line.substr(equal + 1);
        return true;
    }

    Vector<String> paths(const String& name)
    {
        return list(name, path_separator());
    }

    Vector<String> library_paths()
    {
    #if defined(_WIN32)
        return paths("PATH");
    #elif defined(__APPLE__)
        return paths("DYLD_LIBRARY_PATH");
    #else
        return paths("LD_LIBRARY_PATH");
    #endif
    }

    String program_name(const String& name)
    {
    #if defined(_WIN32)
        const bool exe = name.size() > 4 && case_insensitive_equal(name.substr(name.size() - 4), ".exe");
        if (!exe)
        {
            return name + ".exe";
        }
    #endif
        return name;
    }

    String find_program(const String& name)
    {
        const String program = program_name(name);
        //(a path: itself)
        if (program.find_first_of("/\\") != String::npos)
        {
            if (Filesystem::is_file(program))
            {
                return program;
            }
            return String();
        }
        for (const String& folder : paths("PATH"))
        {
            const String file = Filesystem::join(folder, program);
            if (Filesystem::is_file(file))
            {
                return file;
            }
        }
        return String();
    }
}
}
