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
#define WINDOW_ERROR_BAD_LENGTH (ERROR_BAD_LENGTH-4);
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
	SQUARE_API std::string win32_get_last_error_as_string();

	SQUARE_API void get_rect_of_window_including_aero(HWND wnd, RECT *rect);

	SQUARE_API void compute_window_size(const unsigned int size_in[2], unsigned int size_out[2]);
	///////////////////////////////////////////////////////////////////////////////////
}
}
}