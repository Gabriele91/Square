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
// The names are local to the model folder: the engine resolves them from there first.
class UniqueNames
{
    std::unordered_set<std::string> m_used;

    static std::string sanitize(const std::string& name)
    {
        std::string out;
        for (const char c : name)
        {
            out += (std::isalnum((unsigned char)c) || c == '_' || c == '-') ? c : '_';
        }
        return out;
    }

public:
    std::string make(const std::string& wanted, const std::string& fallback)
    {
        std::string base = sanitize(wanted);
        if (base.empty()) base = sanitize(fallback);
        std::string name = base;
        for (int n = 2; !m_used.insert(name).second; ++n)
        {
            name = base + "_" + std::to_string(n);
        }
        return name;
    }
};
