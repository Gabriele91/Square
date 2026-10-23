//
//  Environment.h
//  Square
//
//  The environment of the process: its variables (read, as a flag, a number, a list; set,
//  removed), all of them (the environment of a process started), a line NAME=VALUE (env.txt, the
//  output of a shell), the folders of a variable of folders (PATH), a program in the PATH.
//  Windows: UTF-8, the variables of the process and of its C runtime together.
//  (The heap reads SQUARE_HEAP before it is made: std::getenv, not this.)
//
#pragma once
#include "Square/Config.h"

namespace Square
{
namespace Environment
{
    //a variable (empty: not set), else a value
    SQUARE_API String variable(const String& name);
    SQUARE_API String variable(const String& name, const String& fallback);
    //set (even empty)
    SQUARE_API bool has(const String& name);
    //its value is this one (case insensitive: forward, ON, 1)
    SQUARE_API bool is(const String& name, const String& value);
    //as a number (not set, not a number: the fallback)
    SQUARE_API long   integer(const String& name, long fallback);
    SQUARE_API double real(const String& name, double fallback);
    //as a list (its separator: ','), the empty items out
    SQUARE_API Vector<String> list(const String& name, char separator);

    //set (the process and the ones it starts), removed
    SQUARE_API bool set(const String& name, const String& value);
    SQUARE_API bool remove(const String& name);
    //all of them (NAME: VALUE)
    SQUARE_API Map<String, String> all();
    //a line NAME=VALUE (a \r at its end out): false, not one (empty, a #comment, no name)
    SQUARE_API bool parse(String line, String& name, String& value);

    //the separator of a variable of folders (';' on Windows, ':' elsewhere), its folders
    SQUARE_API char path_separator();
    SQUARE_API Vector<String> paths(const String& name);
    //the folders of the shared libraries: Windows PATH, Linux LD_LIBRARY_PATH, macOS
    //DYLD_LIBRARY_PATH
    SQUARE_API Vector<String> library_paths();
    //a program: its name on this system (.exe on Windows), its file in the PATH (a path given:
    //itself; empty: none)
    SQUARE_API String program_name(const String& name);
    SQUARE_API String find_program(const String& name);
}
}
