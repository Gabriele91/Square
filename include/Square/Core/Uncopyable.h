//
//  Square
//
//  Created by Gabriele Di Bari on 18/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"

namespace Square
{
    class SQUARE_API Uncopyable
    {
    protected:
        Uncopyable() = default;
        ~Uncopyable() = default;

        Uncopyable(const Uncopyable&) = delete;
        Uncopyable& operator=(const Uncopyable&) = delete;

        Uncopyable(const Uncopyable&&) = delete;
        Uncopyable& operator=(const Uncopyable&&) = delete;
    };
}
