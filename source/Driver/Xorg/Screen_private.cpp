//
//  Square
//
//  Created by Gabriele on 14/08/16.
//  Copyright � 2016 Gabriele. All rights reserved.
//
#include "Screen_private.h"

namespace Square
{
namespace Video
{
    ScreenXorg::ScreenXorg(unsigned int screen_id,XScreen* xscreen)
    {
        m_xscreen   = xscreen;
        m_screen_id = screen_id;
    }
}
}