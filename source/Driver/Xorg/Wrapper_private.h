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

///////////////////////////////////////////////////
#include <vector>
#include <functional>
#include <unordered_map>
///////////////////////////////////////////////////
// Xorg
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <X11/extensions/Xrandr.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/Xinerama.h>
#include <X11/extensions/xf86vmode.h>
#include <GL/gl.h>
#include <GL/glx.h>

///////////////////////////////////////////////////
// OpenGL Context
#define GLX_CONTEXT_MAJOR_VERSION_ARB       0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB       0x2092
typedef GLXContext(*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

///////////////////////////////////////////////////
//Xorg alias
typedef  ::Screen XScreen;
typedef  ::Window XWindow;

///////////////////////////////////////////////////
// Define
#ifdef None
	#define X11None 0L
	#undef None
#else
	#define X11None 0L
#endif
#ifndef Button6
	#define Button6 6
#endif
#ifndef Button7
	#define Button7 7
#endif
#ifndef Button8
	#define Button8 8
#endif

#define X11_FULL_SCREEN_WINDOW_ATTRIBUTE \
ExposureMask       \
| KeyPressMask       \
| KeyReleaseMask     \
| ButtonPressMask    \
| ButtonReleaseMask  \
  | StructureNotifyMask

#define X11_WINDOW_ATTRIBUTE \
ExposureMask       \
| KeyPressMask       \
| KeyReleaseMask     \
| ButtonPressMask    \
| ButtonReleaseMask  \
| PointerMotionMask  \
| StructureNotifyMask

#define X11_SIZE_HINTS_NO_RESIZE\
  PSize		\
| PMinSize	\
| PMaxSize	\
| USPosition	

#define X11_SIZE_HINTS_RESIZE\
  USSize	\
| PMinSize	\
| PMaxSize	\
| USPosition


#define FILTER_X11_KEY_SIZE (0x01FF)
#define FILTER_X11_KEY(X) (X & FILTER_X11_KEY_SIZE)

namespace Square
{
namespace Video
{
	//struct declaretion
	struct ScreenXorg;
	struct WrapperContext;
	struct InputKeyMap;
	struct WindowXorg;
    struct InputXorg;

    // Map input
	struct InputKeyMap
	{
		unsigned int  m_scancodes[512];
		KeyboardEvent m_keyboard[512];
	};

    // Context
	struct WrapperContext
	{
		Display*				                     m_xdisplay{ nullptr };
		XContext				                     m_xcontext;
		std::vector<ScreenXorg>	                     m_screens;
		std::unordered_map<WindowXorg*, WindowXorg*> m_windows;
		std::unordered_map<InputXorg*, InputXorg*>   m_input;
		//key map
		InputKeyMap			               m_key_map;
    };    
	extern WrapperContext s_os_context;
}
}