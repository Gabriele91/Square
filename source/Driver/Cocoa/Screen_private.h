//
//  Square
//
//  Created by Gabriele on 14/08/16.
//  Copyright � 2016 Gabriele. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Driver/Window.h"
#include "Square/Driver/Input.h"
#include "Wrapper_private.h"

namespace Square
{
namespace Video
{
namespace Cocoa
{
    class SQUARE_API ScreenCocoa
    {
    public:
        
        uint32_t m_id{ 0 };
        uint32_t m_unit_number{ 0 };
        float    m_size[2]{ 0, 0 };
        
        ScreenCocoa();
        
        ScreenCocoa(uint32_t id, uint32_t unit_number, const CGSize size);
        
        void get_position(unsigned int size[2]);
        
        struct ScreenMode
        {
            size_t width;
            size_t height;
            size_t depth;
        };
        
        static size_t display_bits_per_pixel_for_mode(CGDisplayModeRef mode);
        
        CGDisplayModeRef best_match_for_mode( ScreenMode screenMode );
    };
}
}
}
