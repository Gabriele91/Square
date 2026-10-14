//
//  Square
//
//  Created by Gabriele Di Bari on 18/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include <string>
#include <unordered_set>
#include <cctype>

// Resource names taken from the model (Blender object/material/image names), made safe for a
// file name and unique: a "_2", "_3"... suffix is added only when two names really clash.
// Lowercase: the same file on any filesystem (Linux, macOS case sensitive: "Box.mat" is not
// "box.mat"). The names are local to the model folder: the engine resolves them from there first.
class UniqueNames
{
    Square::UnorderedSet<Square::String> m_used;

    static Square::String sanitize(const Square::String& name)
    {
        Square::String out;
        out.reserve(name.size());
        for (const char c : name)
        {
            const unsigned char letter = (unsigned char)c;
            out += (std::isalnum(letter) || c == '_' || c == '-') ? char(std::tolower(letter)) : '_';
        }
        return out;
    }

public:
    Square::String make(const Square::String& wanted, const Square::String& fallback)
    {
        Square::String base = sanitize(wanted);
        if (base.empty()) base = sanitize(fallback);
        Square::String name = base;
        for (int n = 2; !m_used.insert(name).second; ++n)
        {
            name = base + "_" + Square::to_string(n);
        }
        return name;
    }
};
