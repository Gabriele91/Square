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
#include <windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#include "Screen_private.h"
#include "Window_private.h"
#include "Input_private.h"

#define WINDOW_CLASS_NAME TEXT("Square")
#define SQUARE_WINDOW_STYLE WS_TILEDWINDOW
#define SQUARE_WINDOW_NORESIZE_STYLE (WS_TILEDWINDOW ^ WS_THICKFRAME)
#define SQUARE_FULLSCREEN_STYLE (WS_POPUP | WS_VISIBLE)

namespace Square
{
namespace Video
{
namespace Win32
{
	///////////////////////////////////////////////////////////////////////////////////
	SQUARE_API String win32_get_last_error_as_string();

	SQUARE_API void get_rect_of_window_including_aero(HWND wnd, RECT *rect);

	//the size of a window (its borders, its title) of a client area: of its style and its DPI (of
	//its monitor; none: of the system); dpi: another one (WM_DPICHANGED), 0: of the window
	SQUARE_API void compute_window_size(const unsigned int size_in[2], unsigned int size_out[2], HWND window = nullptr, UINT dpi = 0);
	//a window just made: of the client area asked (its borders of its style and of the DPI of its
	//monitor, not known before it is made)
	SQUARE_API void fit_window_size(HWND window, const unsigned int size[2]);
	///////////////////////////////////////////////////////////////////////////////////
}
}
}