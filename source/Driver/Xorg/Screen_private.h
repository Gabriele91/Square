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

///////////////////////////////////////////////////

namespace Square
{
namespace Video
{
	struct ScreenXorg
	{
		unsigned int m_screen_id{ 0 };
		XScreen* m_xscreen		{ nullptr };
		////////////////////////////////////////
		ScreenXorg(unsigned int screen_id,XScreen* xscreen);
    };
}
}