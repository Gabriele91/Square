//
//  Square
//
//  Created by Gabriele on 14/08/16.
//  Copyright � 2016 Gabriele. All rights reserved.
//
#include "Square/Config.h"
#include "Square/Driver/Window.h"
#include "Square/Driver/Input.h"
#include "Wrapper_private.h"
#include <unordered_map>

typedef HRESULT(WINAPI * DwmIsCompositionEnabledFunction)(__out BOOL* isEnabled);
typedef HRESULT(WINAPI *DwmGetWindowAttributeFunction) (__in  HWND hwnd, __in  DWORD dwAttribute, __out PVOID pvAttribute, DWORD cbAttribute);
////////////////////////////////////////////////////////////////////////////////////
// Trick, GPU selection OpenGL
struct OptimusVariables
{
	OptimusVariables()
	{
		HMODULE hModule = GetModuleHandle(nullptr);
		if (hModule != nullptr)
		{
			NvOptimusEnablement = (uint32_t*)GetProcAddress(hModule, "NvOptimusEnablement");
			AmdPowerXpressRequestHighPerformance = (int*)GetProcAddress(hModule, "AmdPowerXpressRequestHighPerformance");
		}
	}

	bool enable()
	{
		if (NvOptimusEnablement && AmdPowerXpressRequestHighPerformance)
		{
			(*NvOptimusEnablement) = 1;
			(*AmdPowerXpressRequestHighPerformance) = 1;
			return true;
		}
		return false;
	}

	bool disable()
	{
		if (NvOptimusEnablement && AmdPowerXpressRequestHighPerformance)
		{
			(*NvOptimusEnablement) = 0;
			(*AmdPowerXpressRequestHighPerformance) = 0;
			return true;
		}
		return false;
	}

protected:
	uint32_t* NvOptimusEnablement = nullptr;
	int* AmdPowerXpressRequestHighPerformance = nullptr;
};
////////////////////////////////////////////////////////////////////////////////////

namespace Square
{
namespace Video
{
	///////////////////////////////////////////////////////////////////////////////////
	// Wrapper
	namespace Win32
	{
		std::string win32_get_last_error_as_string()
		{
			//Get the error message, if any.
			DWORD error_message_id = ::GetLastError();
			//no error
			if (!error_message_id) return {};
			//get error string
			LPSTR message_buffer = nullptr;
			size_t size = FormatMessageA
			(
				FORMAT_MESSAGE_ALLOCATE_BUFFER
				| FORMAT_MESSAGE_FROM_SYSTEM
				| FORMAT_MESSAGE_IGNORE_INSERTS
				, NULL
				, error_message_id
				, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
				, (LPSTR)&message_buffer
				, 0
				, NULL
			);
			//C++ message
			std::string message(message_buffer, size);
			//free the buffer.
			LocalFree(message_buffer);
			//return msg
			return message;
		}

		void get_rect_of_window_including_aero(HWND wnd, RECT *rect)
		{
			// default to old way of getting the window rectandle
			::GetWindowRect(wnd, rect);
			// Load the dll and keep the handle to it
			// must load dynamicaly because this dll exists only in vista -- not in xp.
			// if this is running on XP then use old way.
			auto dwmapi_dll_handle = LoadLibrary("dwmapi.dll");
			// not on Vista/Windows7 so no aero so no need to account for aero.  could use GetVersion to determine this faster.
			if (dwmapi_dll_handle)
			{
				HRESULT a_result = S_OK;
				BOOL is_enabled = false;
				if (auto dwm_is_composition_enabled = (DwmIsCompositionEnabledFunction)::GetProcAddress(dwmapi_dll_handle, "DwmIsCompositionEnabled"))
				{
					a_result = dwm_is_composition_enabled(&is_enabled);
				}
				//
				if (SUCCEEDED(a_result) && is_enabled)
				{
					if (auto dwm_get_window_attribute = (DwmGetWindowAttributeFunction)::GetProcAddress(dwmapi_dll_handle, "DwmGetWindowAttribute"))
					{
						a_result = dwm_get_window_attribute(wnd, DWMWA_EXTENDED_FRAME_BOUNDS, rect, sizeof(RECT));
						// hopefully we're ok either way
						//if (SUCCEEDED(a_result)) dwm_get_window_attribute( wnd, DWMWA_EXTENDED_FRAME_BOUNDS, rect, sizeof(RECT));
					}
				}
				FreeLibrary(dwmapi_dll_handle);
			}
		}

		void compute_window_size(const unsigned int size_in[2], unsigned int size_out[2])
		{
			RECT window_rect
			{
				  0
				, 0
				, LONG(size_in[0]) // right = Width
				, LONG(size_in[1]) // bottom = Height
			};
			AdjustWindowRectEx(&window_rect, SQUARE_WINDOW_STYLE, true, 0);
			//calc size window
			size_out[0] = window_rect.right - window_rect.left;
			size_out[1] = window_rect.bottom - window_rect.top - WINDOW_ERROR_BAD_LENGTH;
		}
	}
	////////////////////////////////////////////////////////////////////////////////////
	struct InputKeyMap
	{
		unsigned int  m_scancodes[512];
		KeyboardEvent m_keyboard[512];
	};

	struct WrapperContext
	{
		std::vector<Win32::ScreenWin32>			           m_screens;
		std::unordered_map<Win32::WindowWin32*, Win32::WindowWin32*> m_windows;
		std::unordered_map<Win32::InputWin32*, Win32::InputWin32*>   m_input;
		//key map
		InputKeyMap						           m_key_map;
	};

	static WrapperContext s_os_context;
	////////////////////////////////////////////////////////////////////////////////////
	// this is the main message handler for the program	
	LRESULT CALLBACK win32_event_wrapper(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static Win32::InputWin32* input_create(Window* wnd, Input* input);
	static void input_delete(Win32::InputWin32*& in);
	static Win32::WindowWin32* window_create(const WindowInfo& info, Window* window);
	static void	window_delete(Win32::WindowWin32*& window);
	////////////////////////////////////////////////////////////////////////////////////
	// VIDEO
	static bool helper_screen_init()
	{
		//device name
		TCHAR device_name[32]{ 0 };
		ZeroMemory(&device_name, sizeof(device_name));
		//device
		DISPLAY_DEVICE device;
		ZeroMemory(&device, sizeof(DISPLAY_DEVICE));
		device.cb = sizeof(device);

		// Get the primary display in the first pass
		for (short pass = 0; pass != 2; ++pass)
		{
			for (DWORD i = 0; ; ++i)
			{
				//reset name
				TCHAR device_name[32]{ 0 };
				ZeroMemory(&device_name, sizeof(device_name));
				//reset device 
				ZeroMemory(&device, sizeof(DISPLAY_DEVICE));
				device.cb = sizeof(device);
				//get device
				if (!EnumDisplayDevices(NULL, i, &device, 0)) break;
				if (!(device.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)) continue;
				//only primary
				if (pass == 0)  if (!(device.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)) continue;
				else            if (device.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)    continue;
				//save name
				std::memcpy(device_name, device.DeviceName, sizeof(device_name));
				//reset device 
				ZeroMemory(&device, sizeof(DISPLAY_DEVICE));
				device.cb = sizeof(device);
				//for monitor device
				for (DWORD j = 0; ; ++j)
				{
					//get desktop of device
					if (!EnumDisplayDevices(device_name, j, &device, 0)) break;
					if (!(device.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)) continue;
					//is primary
					if (pass == 0)  if (!(device.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)) continue;
					else			if (device.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)    continue;
					//add screen
					s_os_context.m_screens.emplace_back(device_name, device.DeviceName);
				}
				//fake desktop (a device)
				if (!s_os_context.m_screens.size())
				{
					//add screen
					s_os_context.m_screens.emplace_back(device_name);
				}
			}
		}
		//failed
		if (!s_os_context.m_screens.size()) return false;
		//else success
		return true;
	}

	static void helper_input_init()
	{
		memset(s_os_context.m_key_map.m_keyboard, KeyboardEvent::KEY_UNKNOWN, sizeof(s_os_context.m_key_map.m_keyboard));
		memset(s_os_context.m_key_map.m_scancodes, -1, sizeof(s_os_context.m_key_map.m_scancodes));

		s_os_context.m_key_map.m_keyboard[0x00B] = KeyboardEvent::KEY_0;
		s_os_context.m_key_map.m_keyboard[0x002] = KeyboardEvent::KEY_1;
		s_os_context.m_key_map.m_keyboard[0x003] = KeyboardEvent::KEY_2;
		s_os_context.m_key_map.m_keyboard[0x004] = KeyboardEvent::KEY_3;
		s_os_context.m_key_map.m_keyboard[0x005] = KeyboardEvent::KEY_4;
		s_os_context.m_key_map.m_keyboard[0x006] = KeyboardEvent::KEY_5;
		s_os_context.m_key_map.m_keyboard[0x007] = KeyboardEvent::KEY_6;
		s_os_context.m_key_map.m_keyboard[0x008] = KeyboardEvent::KEY_7;
		s_os_context.m_key_map.m_keyboard[0x009] = KeyboardEvent::KEY_8;
		s_os_context.m_key_map.m_keyboard[0x00A] = KeyboardEvent::KEY_9;
		s_os_context.m_key_map.m_keyboard[0x01E] = KeyboardEvent::KEY_A;
		s_os_context.m_key_map.m_keyboard[0x030] = KeyboardEvent::KEY_B;
		s_os_context.m_key_map.m_keyboard[0x02E] = KeyboardEvent::KEY_C;
		s_os_context.m_key_map.m_keyboard[0x020] = KeyboardEvent::KEY_D;
		s_os_context.m_key_map.m_keyboard[0x012] = KeyboardEvent::KEY_E;
		s_os_context.m_key_map.m_keyboard[0x021] = KeyboardEvent::KEY_F;
		s_os_context.m_key_map.m_keyboard[0x022] = KeyboardEvent::KEY_G;
		s_os_context.m_key_map.m_keyboard[0x023] = KeyboardEvent::KEY_H;
		s_os_context.m_key_map.m_keyboard[0x017] = KeyboardEvent::KEY_I;
		s_os_context.m_key_map.m_keyboard[0x024] = KeyboardEvent::KEY_J;
		s_os_context.m_key_map.m_keyboard[0x025] = KeyboardEvent::KEY_K;
		s_os_context.m_key_map.m_keyboard[0x026] = KeyboardEvent::KEY_L;
		s_os_context.m_key_map.m_keyboard[0x032] = KeyboardEvent::KEY_M;
		s_os_context.m_key_map.m_keyboard[0x031] = KeyboardEvent::KEY_N;
		s_os_context.m_key_map.m_keyboard[0x018] = KeyboardEvent::KEY_O;
		s_os_context.m_key_map.m_keyboard[0x019] = KeyboardEvent::KEY_P;
		s_os_context.m_key_map.m_keyboard[0x010] = KeyboardEvent::KEY_Q;
		s_os_context.m_key_map.m_keyboard[0x013] = KeyboardEvent::KEY_R;
		s_os_context.m_key_map.m_keyboard[0x01F] = KeyboardEvent::KEY_S;
		s_os_context.m_key_map.m_keyboard[0x014] = KeyboardEvent::KEY_T;
		s_os_context.m_key_map.m_keyboard[0x016] = KeyboardEvent::KEY_U;
		s_os_context.m_key_map.m_keyboard[0x02F] = KeyboardEvent::KEY_V;
		s_os_context.m_key_map.m_keyboard[0x011] = KeyboardEvent::KEY_W;
		s_os_context.m_key_map.m_keyboard[0x02D] = KeyboardEvent::KEY_X;
		s_os_context.m_key_map.m_keyboard[0x015] = KeyboardEvent::KEY_Y;
		s_os_context.m_key_map.m_keyboard[0x02C] = KeyboardEvent::KEY_Z;

		s_os_context.m_key_map.m_keyboard[0x028] = KeyboardEvent::KEY_APOSTROPHE;
		s_os_context.m_key_map.m_keyboard[0x02B] = KeyboardEvent::KEY_BACKSLASH;
		s_os_context.m_key_map.m_keyboard[0x033] = KeyboardEvent::KEY_COMMA;
		s_os_context.m_key_map.m_keyboard[0x00D] = KeyboardEvent::KEY_EQUAL;
		s_os_context.m_key_map.m_keyboard[0x029] = KeyboardEvent::KEY_GRAVE_ACCENT;
		s_os_context.m_key_map.m_keyboard[0x01A] = KeyboardEvent::KEY_LEFT_BRACKET;
		s_os_context.m_key_map.m_keyboard[0x00C] = KeyboardEvent::KEY_MINUS;
		s_os_context.m_key_map.m_keyboard[0x034] = KeyboardEvent::KEY_PERIOD;
		s_os_context.m_key_map.m_keyboard[0x01B] = KeyboardEvent::KEY_RIGHT_BRACKET;
		s_os_context.m_key_map.m_keyboard[0x027] = KeyboardEvent::KEY_SEMICOLON;
		s_os_context.m_key_map.m_keyboard[0x035] = KeyboardEvent::KEY_SLASH;
		s_os_context.m_key_map.m_keyboard[0x056] = KeyboardEvent::KEY_WORLD_2;

		s_os_context.m_key_map.m_keyboard[0x00E] = KeyboardEvent::KEY_BACKSPACE;
		s_os_context.m_key_map.m_keyboard[0x153] = KeyboardEvent::KEY_DELETE;
		s_os_context.m_key_map.m_keyboard[0x14F] = KeyboardEvent::KEY_END;
		s_os_context.m_key_map.m_keyboard[0x01C] = KeyboardEvent::KEY_ENTER;
		s_os_context.m_key_map.m_keyboard[0x001] = KeyboardEvent::KEY_ESCAPE;
		s_os_context.m_key_map.m_keyboard[0x147] = KeyboardEvent::KEY_HOME;
		s_os_context.m_key_map.m_keyboard[0x152] = KeyboardEvent::KEY_INSERT;
		s_os_context.m_key_map.m_keyboard[0x15D] = KeyboardEvent::KEY_MENU;
		s_os_context.m_key_map.m_keyboard[0x151] = KeyboardEvent::KEY_PAGE_DOWN;
		s_os_context.m_key_map.m_keyboard[0x149] = KeyboardEvent::KEY_PAGE_UP;
		s_os_context.m_key_map.m_keyboard[0x045] = KeyboardEvent::KEY_PAUSE;
		s_os_context.m_key_map.m_keyboard[0x146] = KeyboardEvent::KEY_PAUSE;
		s_os_context.m_key_map.m_keyboard[0x039] = KeyboardEvent::KEY_SPACE;
		s_os_context.m_key_map.m_keyboard[0x00F] = KeyboardEvent::KEY_TAB;
		s_os_context.m_key_map.m_keyboard[0x03A] = KeyboardEvent::KEY_CAPS_LOCK;
		s_os_context.m_key_map.m_keyboard[0x145] = KeyboardEvent::KEY_NUM_LOCK;
		s_os_context.m_key_map.m_keyboard[0x046] = KeyboardEvent::KEY_SCROLL_LOCK;
		s_os_context.m_key_map.m_keyboard[0x03B] = KeyboardEvent::KEY_F1;
		s_os_context.m_key_map.m_keyboard[0x03C] = KeyboardEvent::KEY_F2;
		s_os_context.m_key_map.m_keyboard[0x03D] = KeyboardEvent::KEY_F3;
		s_os_context.m_key_map.m_keyboard[0x03E] = KeyboardEvent::KEY_F4;
		s_os_context.m_key_map.m_keyboard[0x03F] = KeyboardEvent::KEY_F5;
		s_os_context.m_key_map.m_keyboard[0x040] = KeyboardEvent::KEY_F6;
		s_os_context.m_key_map.m_keyboard[0x041] = KeyboardEvent::KEY_F7;
		s_os_context.m_key_map.m_keyboard[0x042] = KeyboardEvent::KEY_F8;
		s_os_context.m_key_map.m_keyboard[0x043] = KeyboardEvent::KEY_F9;
		s_os_context.m_key_map.m_keyboard[0x044] = KeyboardEvent::KEY_F10;
		s_os_context.m_key_map.m_keyboard[0x057] = KeyboardEvent::KEY_F11;
		s_os_context.m_key_map.m_keyboard[0x058] = KeyboardEvent::KEY_F12;
		s_os_context.m_key_map.m_keyboard[0x064] = KeyboardEvent::KEY_F13;
		s_os_context.m_key_map.m_keyboard[0x065] = KeyboardEvent::KEY_F14;
		s_os_context.m_key_map.m_keyboard[0x066] = KeyboardEvent::KEY_F15;
		s_os_context.m_key_map.m_keyboard[0x067] = KeyboardEvent::KEY_F16;
		s_os_context.m_key_map.m_keyboard[0x068] = KeyboardEvent::KEY_F17;
		s_os_context.m_key_map.m_keyboard[0x069] = KeyboardEvent::KEY_F18;
		s_os_context.m_key_map.m_keyboard[0x06A] = KeyboardEvent::KEY_F19;
		s_os_context.m_key_map.m_keyboard[0x06B] = KeyboardEvent::KEY_F20;
		s_os_context.m_key_map.m_keyboard[0x06C] = KeyboardEvent::KEY_F21;
		s_os_context.m_key_map.m_keyboard[0x06D] = KeyboardEvent::KEY_F22;
		s_os_context.m_key_map.m_keyboard[0x06E] = KeyboardEvent::KEY_F23;
		s_os_context.m_key_map.m_keyboard[0x076] = KeyboardEvent::KEY_F24;
		s_os_context.m_key_map.m_keyboard[0x038] = KeyboardEvent::KEY_LEFT_ALT;
		s_os_context.m_key_map.m_keyboard[0x01D] = KeyboardEvent::KEY_LEFT_CONTROL;
		s_os_context.m_key_map.m_keyboard[0x02A] = KeyboardEvent::KEY_LEFT_SHIFT;
		s_os_context.m_key_map.m_keyboard[0x15B] = KeyboardEvent::KEY_LEFT_SUPER;
		s_os_context.m_key_map.m_keyboard[0x137] = KeyboardEvent::KEY_PRINT_SCREEN;
		s_os_context.m_key_map.m_keyboard[0x138] = KeyboardEvent::KEY_RIGHT_ALT;
		s_os_context.m_key_map.m_keyboard[0x11D] = KeyboardEvent::KEY_RIGHT_CONTROL;
		s_os_context.m_key_map.m_keyboard[0x036] = KeyboardEvent::KEY_RIGHT_SHIFT;
		s_os_context.m_key_map.m_keyboard[0x15C] = KeyboardEvent::KEY_RIGHT_SUPER;
		s_os_context.m_key_map.m_keyboard[0x150] = KeyboardEvent::KEY_DOWN;
		s_os_context.m_key_map.m_keyboard[0x14B] = KeyboardEvent::KEY_LEFT;
		s_os_context.m_key_map.m_keyboard[0x14D] = KeyboardEvent::KEY_RIGHT;
		s_os_context.m_key_map.m_keyboard[0x148] = KeyboardEvent::KEY_UP;

		s_os_context.m_key_map.m_keyboard[0x052] = KeyboardEvent::KEY_KP_0;
		s_os_context.m_key_map.m_keyboard[0x04F] = KeyboardEvent::KEY_KP_1;
		s_os_context.m_key_map.m_keyboard[0x050] = KeyboardEvent::KEY_KP_2;
		s_os_context.m_key_map.m_keyboard[0x051] = KeyboardEvent::KEY_KP_3;
		s_os_context.m_key_map.m_keyboard[0x04B] = KeyboardEvent::KEY_KP_4;
		s_os_context.m_key_map.m_keyboard[0x04C] = KeyboardEvent::KEY_KP_5;
		s_os_context.m_key_map.m_keyboard[0x04D] = KeyboardEvent::KEY_KP_6;
		s_os_context.m_key_map.m_keyboard[0x047] = KeyboardEvent::KEY_KP_7;
		s_os_context.m_key_map.m_keyboard[0x048] = KeyboardEvent::KEY_KP_8;
		s_os_context.m_key_map.m_keyboard[0x049] = KeyboardEvent::KEY_KP_9;
		s_os_context.m_key_map.m_keyboard[0x04E] = KeyboardEvent::KEY_KP_ADD;
		s_os_context.m_key_map.m_keyboard[0x053] = KeyboardEvent::KEY_KP_DECIMAL;
		s_os_context.m_key_map.m_keyboard[0x135] = KeyboardEvent::KEY_KP_DIVIDE;
		s_os_context.m_key_map.m_keyboard[0x11C] = KeyboardEvent::KEY_KP_ENTER;
		s_os_context.m_key_map.m_keyboard[0x037] = KeyboardEvent::KEY_KP_MULTIPLY;
		s_os_context.m_key_map.m_keyboard[0x04A] = KeyboardEvent::KEY_KP_SUBTRACT;

		for (unsigned int i_scancode = 0; i_scancode != 512; ++i_scancode)
		{
			if (s_os_context.m_key_map.m_keyboard[i_scancode] > 0)
				if (s_os_context.m_key_map.m_keyboard[i_scancode] != KeyboardEvent::KEY_UNKNOWN)
					s_os_context.m_key_map.m_scancodes[s_os_context.m_key_map.m_keyboard[i_scancode]] = i_scancode;
		}
	}

	void init()
	{
		//Add a window class
		WNDCLASS wmd_class;
		wmd_class.cbClsExtra = 0;
		wmd_class.cbWndExtra = 0;
		wmd_class.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wmd_class.hCursor = LoadCursor(NULL, IDC_ARROW);
		wmd_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wmd_class.hInstance = (HINSTANCE)GetModuleHandle(NULL);
		wmd_class.lpfnWndProc = win32_event_wrapper;
		wmd_class.lpszClassName = WINDOW_CLASS_NAME;
		wmd_class.lpszMenuName = 0;
		wmd_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		// Register that class with the Windows O/S..
		RegisterClass(&wmd_class);
		// Get all screen
		helper_screen_init();
		// init input map
		helper_input_init();
	}

	void close()
	{
		//remove all inputs/windows 
		while (s_os_context.m_input.size())   s_os_context.m_input.begin()->second->m_input_ref->destoy();
		while (s_os_context.m_windows.size()) s_os_context.m_windows.begin()->second->m_window_ref->destoy();
		//remove all screen
		s_os_context.m_screens.clear();
		//Delete window class
		UnregisterClass(WINDOW_CLASS_NAME, (HINSTANCE)GetModuleHandle(NULL));
	}
	
	////////////////////////////////////////////////////////////////////////////////////
	//	SCREEN
	unsigned int Screen::count()
	{
		return unsigned int(s_os_context.m_screens.size());
	}

	Screen	Screen::by_index(unsigned int index)
	{
		Screen output;
		output.m_native = &s_os_context.m_screens[index];
		return output;
	}

	Screen::Screen() { /* void */ }

	Screen::~Screen() { /* void */ }

	void* Screen::conteiner() const
	{
		return m_native;
	}

	void Screen::get_size(unsigned int& width, unsigned int& height)  const
	{
		((Win32::ScreenWin32*)m_native)-> /*get_size*/ get_monitor_size(width, height);
	}	

	////////////////////////////////////////////////////////////////////////////////////
	// Window	
	static Win32::WindowWin32* gl_window_create(const WindowInfo& info)
	{
		//////////////////////////////////////////////////////////////////////
		switch (info.m_context.m_gpu_type)
		{
		case ContextInfo::GPU_TYPE_LOW:
			OptimusVariables().disable();
		break;
		case ContextInfo::GPU_TYPE_HIGTH:
			OptimusVariables().enable();
		break;
		// Default: not set
		default:
		case ContextInfo::GPU_TYPE_DEFAULT:
		break;
		}
		//////////////////////////////////////////////////////////////////////
		unsigned int last_window_real_size[2] = { 0,0 };
		Win32::compute_window_size(info.m_size, last_window_real_size);
		//screen position
		int sc_position[2]{ 0,0 };
		//screen size
		unsigned int sc_size[2]{ 0,0 };
		//to screen position
		if (info.m_screen)
		{
			((Win32::ScreenWin32*)info.m_screen->conteiner())->get_position(sc_position[0], sc_position[1]);
			((Win32::ScreenWin32*)info.m_screen->conteiner())->get_monitor_size(sc_size[0], sc_size[1]);
		}
		else
		{
			sc_size[0] = GetSystemMetrics(SM_CXSCREEN);
			sc_size[1] = GetSystemMetrics(SM_CYSCREEN);
		}
		//calc center
		int left = (sc_size[0] - last_window_real_size[0])  / 2 + sc_position[0];
		int top  = (sc_size[1] - last_window_real_size[1]) / 2 + sc_position[1];
		//////////////////////////////////////////////////////////////////////
		HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);		
		//////////////////////////////////////////////////////////////////////
		HWND hWnd = CreateWindow
		(
			WINDOW_CLASS_NAME,
			info.m_title.c_str(),
			info.m_resize
			? SQUARE_WINDOW_STYLE
			: SQUARE_WINDOW_NORESIZE_STYLE,
			left,  //x
			top,   //y
			last_window_real_size[0], //width
			last_window_real_size[1],//height
			NULL,
			NULL,
			hInstance,
			NULL
		);
		//////////////////////////////////////////////////////////////////////
		// create PFD:
		static	PIXELFORMATDESCRIPTOR pfd =				// pfd Tells Windows How We Want Things To Be
		{
			sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
			1,											// Version Number
			PFD_DRAW_TO_WINDOW |						// Format Must Support Window
			PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
			PFD_DOUBLEBUFFER |							// Must Support Double Buffering
			PFD_SWAP_EXCHANGE,
			PFD_TYPE_RGBA,								// Request An RGBA Format
			BYTE(info.m_context.m_color), 				// Select Our Color Depth
			0, 0, 0, 0, 0, 0,							// Color Bits Ignored
			0,											// No Alpha Resource
			0,											// Shift Bit Ignored
			0,											// No Accumulation Resource
			0, 0, 0, 0,									// Accumulation Bits Ignored
			BYTE(info.m_context.m_depth),			    // Z-Resource (Depth Resource)
			BYTE(info.m_context.m_stencil),				// Stencil Resource
			0,											// No Auxiliary Resource
			PFD_MAIN_PLANE,								// Main Drawing Layer
			0,											// Reserved
			0, 0, 0										// Layer Masks Ignored
		};
		// Add support composition 
#if WINDOW_XP_SUPPORT
		OSVERSIONINFO osvi = { 0 };
		if (GetVersionEx(&osvi))
		if (osvi.dwMajorVersion > 6 || (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion >= 0))
			pfd.dwFlags |= PFD_SUPPORT_COMPOSITION;
#else 
		pfd.dwFlags |= PFD_SUPPORT_COMPOSITION;
#endif
		//////////////////////////////////////////////////////////////////////
		//get device context
		HDC hDC = GetDC(hWnd);
		//set pdf 
		int cs_pixel_format = ChoosePixelFormat(hDC, &pfd); //todo, AA
		SetPixelFormat(hDC, cs_pixel_format, &pfd);
		//OpenGL Context (BASE)
		HGLRC hRCDefault = wglCreateContext(hDC);
		wglMakeCurrent(hDC, hRCDefault);		
		//final hRC
		HGLRC hRC = hRCDefault;
		//Get wglCreateContextAttribsARB
		typedef HGLRC(APIENTRY * PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC hDC, HGLRC hShareContext, const int *attribList);
		static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
		//new RC if wglCreateContextAttribsARB exits
		if (wglCreateContextAttribsARB)
		{
			//OpenGL Context 
			#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
			#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
			#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
			#define WGL_CONTEXT_FLAGS_ARB                   0x2094
			#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126
			const int attribs[] =
			{
				WGL_CONTEXT_MAJOR_VERSION_ARB, (const int)info.m_context.m_version[0],
				WGL_CONTEXT_MINOR_VERSION_ARB, (const int)info.m_context.m_version[1],
				NULL
			};
			//overload
			hRC = wglCreateContextAttribsARB(hDC, 0, attribs);
			//disable
			wglMakeCurrent(hDC, NULL);
			//remove default OGL context
			wglDeleteContext(hRCDefault);
		}
		//set new context
		wglMakeCurrent(hDC, hRC);
		//return opengl conteinter
		Win32::WindowGL* wnd_gl = new Win32::WindowGL(hDC, hRC, hWnd, info);
		//fullscreen
		if (wnd_gl->m_info.m_fullscreen)
		{
			wnd_gl->m_info.m_fullscreen = false;
			wnd_gl->m_info.m_fullscreen = wnd_gl->enable_fullscreen(true);
		}
		//return
		return  wnd_gl;
	}	
	
	static Win32::WindowWin32* dx_window_create(const WindowInfo& info)
	{		
		//////////////////////////////////////////////////////////////////////
		unsigned int last_window_real_size[2] = { 0,0 };
		Win32::compute_window_size(info.m_size, last_window_real_size);
		//screen position
		int sc_position[2]{ 0,0 };
		//screen size
		unsigned int sc_size[2]{ 0,0 };
		//to screen position
		if (info.m_screen)
		{
			((Win32::ScreenWin32*)info.m_screen->conteiner())->get_position(sc_position[0], sc_position[1]);
			((Win32::ScreenWin32*)info.m_screen->conteiner())->get_monitor_size(sc_size[0], sc_size[1]);
		}
		else
		{
			sc_size[0] = GetSystemMetrics(SM_CXSCREEN);
			sc_size[1] = GetSystemMetrics(SM_CYSCREEN);
		}
		//calc center
		int left = (sc_size[0] - last_window_real_size[0]) / 2 + sc_position[0];
		int top = (sc_size[1] - last_window_real_size[1]) / 2 + sc_position[1];
		//////////////////////////////////////////////////////////////////////
		HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);
		//////////////////////////////////////////////////////////////////////
		HWND hWnd = CreateWindow
		(
			WINDOW_CLASS_NAME,
			info.m_title.c_str(),
			info.m_resize
			? SQUARE_WINDOW_STYLE
			: SQUARE_WINDOW_NORESIZE_STYLE,
			left,  //x
			top,   //y
			last_window_real_size[0], //width
			last_window_real_size[1],//height
			NULL,
			NULL,
			hInstance,
			NULL
		);
		//return new instance
		return (Win32::WindowWin32*)new Win32::WindowDX(GetDC(hWnd), hWnd, info);
	}
	
	static Win32::WindowWin32* window_create(const WindowInfo& info, Window* window)
	{
		//output var
		Win32::WindowWin32* out_wnd = nullptr;
		//select type of window
		switch (info.m_context.m_type)
		{
			case ContextInfo::CTX_OPENGL_ES:
			case ContextInfo::CTX_OPENGL:  
				out_wnd =  gl_window_create(info);
			break;
			case ContextInfo::CTX_DIRECTX: 
				out_wnd = dx_window_create(info);
			break;
			default: 
				out_wnd = nullptr;
			break;
		}
		//push to list
		if (out_wnd)
		{
			//ref to conteiner
			out_wnd->m_window_ref = window;
			//save
			s_os_context.m_windows[out_wnd] = out_wnd;
			//show window
			ShowWindow((HWND)out_wnd->narive(), SW_SHOW);
		}
		//return 
		return out_wnd;
	}

	static void	window_delete(Win32::WindowWin32*& window)
	{
		//search
		auto it_wnd = s_os_context.m_windows.find(window);
		//remove
		if (it_wnd != s_os_context.m_windows.end())
		{
			s_os_context.m_windows.erase(it_wnd);
		}
		//window input
		Win32::InputWin32*  wnd_input = (Win32::InputWin32*)GetWindowLongPtr((HWND)window->narive(), GWLP_USERDATA);
		//delete input
		if (wnd_input)
		{
			input_delete(wnd_input);
		}
		//dealloc
		delete window;
		//to null
		window = nullptr;
	}
	
	//init
	Window::Window(const WindowInfo& info)
	{
		m_native = window_create(info, this);
	}

	Window::~Window()
	{
		destoy();
	}
	
	//native
	void* Window::native() const
	{
		//cast
		const auto* window = (const Win32::WindowWin32*)m_native;
		//
		return window->narive();
	}
	
	void* Window::conteiner() const
	{
		return m_native;
	}

	DeviceResources*  Window::device() const
	{
		//cast
		const auto* window = (const Win32::WindowWin32*)m_native;
		//get
		return window->device();
	}

	bool Window::valid() const
	{
		return m_native != nullptr;
	}

	void Window::destoy()
	{
		if (m_native)
		{
			//cast
			auto* window = (Win32::WindowWin32*)m_native;
			window_delete(window);
			m_native = (void*)window;
		}
	}
	//actions
	void Window::clear()
	{
		//none
	}

	void Window::swap()
	{
		//cast
		const auto* window = (const Win32::WindowWin32*)m_native;
		//
		window->swap();
	}

	void Window::acquire_context()
	{
		//cast
		const auto* window = (const Win32::WindowWin32*)m_native;
		//
		window->acquire_context();
	}

	//info
	bool   Window::is_fullscreen()
	{
		//cast
		const auto* window = (const Win32::WindowWin32*)m_native;
		//
		return window->is_fullscreen();
	}

	bool   Window::is_resizable()
	{
		//cast
		const auto* window = (const Win32::WindowWin32*)m_native;
		//
		return window->is_resizable();
	}

	void   Window::get_size(unsigned int& w, unsigned int& h)
	{
		//cast
		const auto* window = (const Win32::WindowWin32*)m_native;
		//
		unsigned int size[2];
		window->get_size(size);
		w = size[0];
		h = size[1];
	}

	void   Window::get_position(int& x, int& y)
	{
		//cast
		const auto* window = (const Win32::WindowWin32*)m_native;
		//
		int pos[2];
		window->get_position(pos);
		x = pos[0];
		y = pos[1];

	}
	
	//set
	void   Window::set_size(unsigned int w, unsigned int h)
	{
		//cast
		auto* window = (Win32::WindowWin32*)m_native;
		//
		unsigned int size[2]{ w, h };
		window->set_size(size);
	}

	void   Window::set_position(int x, int y)
	{
		//cast
		auto* window = (Win32::WindowWin32*)m_native;
		//
		int position[2]{ x,y };
		window->set_position(position);
	}

	void   Window::enable_resize(bool enable)
	{
		//cast
		auto* window = (Win32::WindowWin32*)m_native;
		//
		window->enable_resize(enable);
	}

	void   Window::enable_fullscreen(bool enable)
	{
		//cast
		auto* window = (Win32::WindowWin32*)m_native;
		//
		window->enable_fullscreen(enable);
	}
	
	////////////////////////////////////////////////////////////////////////////////////
	//input
	static Win32::InputWin32* input_create(Window* wnd, Input* input)
	{
		//input hendler
		Win32::InputWin32* in = new Win32::InputWin32();
		//set wnd
		in->m_window_ref = wnd;
		//set ref to conteiner
		in->m_input_ref = input;
		//add to map
		s_os_context.m_input[in] = in;
		//set prob
		SetWindowLongPtr((HWND)wnd->native(), GWLP_USERDATA, (LONG_PTR)in);
		//return
		return in;
	}

	static void input_delete(Win32::InputWin32*& in)
	{
		//search
		auto it_in = s_os_context.m_input.find(in);
		//remove
		if (it_in != s_os_context.m_input.end())
		{
			s_os_context.m_input.erase(it_in);
		}
		//remove prob
		SetWindowLongPtr((HWND)in->m_window_ref->native(), GWLP_USERDATA, 0);
		//dealloc
		delete in;
		//to null
		in = nullptr;
	}
	
	static KeyboardEvent win32_translate_key(WPARAM w_param, LPARAM l_param)
	{
		// The Ctrl keys require special handling (LEFT/RIGHT)
		if (w_param == VK_CONTROL)
		{
			MSG next;
			DWORD time;
			// Right side keys have the extended key bit set
			if (l_param & 0x01000000) return KEY_RIGHT_CONTROL;

			// HACK: Alt Gr sends Left Ctrl and then Right Alt in close sequence
			//       We only want the Right Alt message, so if the next message is
			//       Right Alt we ignore this (synthetic) Left Ctrl message
			time = GetMessageTime();
			if (PeekMessageW(&next, NULL, 0, 0, PM_NOREMOVE))
			{
				if (next.message == WM_KEYDOWN || next.message == WM_SYSKEYDOWN || next.message == WM_KEYUP || next.message == WM_SYSKEYUP)
				{
					if (next.wParam == VK_MENU &&
						(next.lParam & 0x01000000) && next.time == time)
					{
						return KeyboardEvent::KEY_INVALID;
					}
				}
			}

			//else is left ctrl
			return KeyboardEvent::KEY_LEFT_CONTROL;
		}
		//ignore
		if (w_param == VK_PROCESSKEY) return KeyboardEvent::KEY_INVALID;
		//return key
		return s_os_context.m_key_map.m_keyboard[HIWORD(l_param) & 0x1FF];
	}

	static short win32_get_key_mode()
	{
		short mods = 0;

		if (GetKeyState(VK_SHIFT) & (1 << 31))
			mods |= KeyboardModEvent::SHIFT;
		if (GetKeyState(VK_CONTROL) & (1 << 31))
			mods |= KeyboardModEvent::CONTROL;
		if (GetKeyState(VK_MENU) & (1 << 31))
			mods |= KeyboardModEvent::ALT;
		if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & (1 << 31))
			mods |= KeyboardModEvent::SUPER;

		return mods;
	}

	LRESULT CALLBACK win32_event_wrapper(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		//window input
		Win32::InputWin32*  wnd_input = (Win32::InputWin32*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

		if (!wnd_input)
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

		//ref to window
		Win32::WindowWin32*  wnd_window = (Win32::WindowWin32*)wnd_input->m_window_ref->conteiner();

		//process messages
		switch (message)
		{
		case WM_SETFOCUS:
		{ wnd_input->send_window_event(WindowEvent::GET_FOCUS); return 0; }

		case WM_KILLFOCUS:
		{ wnd_input->send_window_event(WindowEvent::LOST_FOCUS); return 0; }

		case WM_SYSCOMMAND:
		{
			switch (wParam & 0xfff0)
			{
			case SC_SCREENSAVE:
			case SC_MONITORPOWER:
				if (wnd_window->is_fullscreen()) { return 0; }
				else break;
			case SC_KEYMENU: return 0;
			}
			break;
		}

		case WM_CLOSE:
		{ wnd_input->send_window_event(WindowEvent::CLOSE); return 0; }

		case WM_CHAR:
		case WM_SYSCHAR:
		case WM_UNICHAR:
		{
			if (message == WM_UNICHAR && wParam == UNICODE_NOCHAR)
			{
				// WM_UNICHAR is not sent by Windows, but is sent by some
				// third-party input method engine
				// Returning TRUE here announces support for this message
				return TRUE;
			}
			wnd_input->send_character_event((unsigned int)wParam, win32_get_key_mode(), (message != WM_SYSCHAR));
			return 0;
		}

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			const KeyboardEvent key = win32_translate_key(wParam, lParam);
			const int scancode = (lParam >> 16) & 0x1ff;
			const ActionEvent action = ((lParam >> 31) & 1) ? ActionEvent::RELEASE : ActionEvent::PRESS;
			const short mods = win32_get_key_mode();

			if (key == KeyboardEvent::KEY_INVALID) break;

			if (action == ActionEvent::RELEASE && wParam == VK_SHIFT)
			{
				// HACK: Release both Shift keys on Shift up event, as when both
				//       are pressed the first release does not emit any event
				// NOTE: The other half of this is in _glfwPlatformPollEvents
				wnd_input->send_keyboard_event(KeyboardEvent::KEY_LEFT_SHIFT, mods, action);
				wnd_input->send_keyboard_event(KeyboardEvent::KEY_RIGHT_SHIFT, mods, action);
			}
			else if (wParam == VK_SNAPSHOT)
			{
				// HACK: Key down is not reported for the Print Screen key
				wnd_input->send_keyboard_event(key, mods, ActionEvent::RELEASE);
				wnd_input->send_keyboard_event(key, mods, ActionEvent::PRESS);
			}
			else wnd_input->send_keyboard_event(key, mods, action);

			break;
		}

		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_XBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_XBUTTONUP:
		{
			MouseButtonEvent button;
			ActionEvent action;

			//button
			if (message == WM_LBUTTONDOWN || message == WM_LBUTTONUP)      button = MouseButtonEvent::MOUSE_BUTTON_LEFT;
			else if (message == WM_RBUTTONDOWN || message == WM_RBUTTONUP) button = MouseButtonEvent::MOUSE_BUTTON_RIGHT;
			else if (message == WM_MBUTTONDOWN || message == WM_MBUTTONUP) button = MouseButtonEvent::MOUSE_BUTTON_MIDDLE;
			else if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1)               button = MouseButtonEvent::MOUSE_BUTTON_4;
			else                                                           button = MouseButtonEvent::MOUSE_BUTTON_5;
			//action
			action = (message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN ||
				message == WM_MBUTTONDOWN || message == WM_XBUTTONDOWN) ? ActionEvent::RELEASE : ActionEvent::PRESS;

			//for all keys
			/*
			for (int i = 0; i < mouse_button_event::MOUSE_BUTTON_LAST; i++)
			{
			if (window->mouseButtons[i] == GLFW_PRESS) break;
			}
			if (i == GLFW_MOUSE_BUTTON_LAST) SetCapture(hWnd);
			*/

			wnd_input->send_mouse_button_event(button, action);

			/*
			for (i = 0; i < GLFW_MOUSE_BUTTON_LAST; i++)
			{
			if (window->mouseButtons[i] == GLFW_PRESS)
			break;
			}
			//relese capture
			if (i == GLFW_MOUSE_BUTTON_LAST) ReleaseCapture();
			*/

			//message acceted
			if (message == WM_XBUTTONDOWN || message == WM_XBUTTONUP) return TRUE;

			//or reject
			return 0;
		}

		case WM_MOUSEMOVE:
		{ wnd_input->send_mouse_move_event(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); return 0; }

		case WM_MOUSEWHEEL:
		{ wnd_input->send_mouse_scroll_event((SHORT)GET_WHEEL_DELTA_WPARAM(wParam) / (double)WHEEL_DELTA); return 0; }

		case WM_MOUSEHWHEEL:
		{ wnd_input->send_mouse_scroll_event((SHORT)GET_WHEEL_DELTA_WPARAM(wParam) / (double)WHEEL_DELTA); return 0; }

		case WM_SIZE:
		{
			if (wParam == SIZE_MINIMIZED) wnd_input->send_window_event(WindowEvent::MINIMIZED);
			if (wParam == SIZE_MAXIMIZED) wnd_input->send_window_event(WindowEvent::MAXIMIZED);
			if (wParam == SIZE_RESTORED)
			{
				//wnd_window->m_info.m_size[0] = LOWORD(lParam);
				//wnd_window->m_info.m_size[1] = HIWORD(lParam);
				wnd_window->get_size(wnd_window->m_info.m_size);
				wnd_input->send_window_event(WindowEvent::RESIZE);
			}
			return 0;
		}

		case WM_MOVE:
		{
			// NOTE: This cannot use LOWORD/HIWORD recommended by MSDN, as
			// those macros do not handle negative window positions correctly
			wnd_window->m_info.m_position[0] = GET_X_LPARAM(lParam);
			wnd_window->m_info.m_position[1] = GET_Y_LPARAM(lParam);
			wnd_input->send_window_move_event(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;
		}

		case WM_ERASEBKGND:
		{ return TRUE; }

		case WM_DPICHANGED:
		{
			RECT* rect = (RECT*)lParam;
			SetWindowPos(
				hWnd,
				HWND_TOP,
				rect->left,
				rect->top,
				rect->right - rect->left,
				rect->bottom - rect->top,
				SWP_NOACTIVATE | SWP_NOZORDER
			);
			break;
		}

		}

		//default return
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	//Input
	Input::Input(Window* wnd)
	{
		m_native = input_create(wnd, this);
	}

	Input::~Input()
	{
		destoy();
	}
	
	//info
	void* Input::conteiner() const
	{
		return m_native;
	}

	bool Input::valid() const
	{
		return m_native != nullptr;
	}

	void Input::destoy()
	{
		if (m_native)
		{
			auto* input = (Win32::InputWin32*)m_native;
			input_delete(input);
			m_native = (void*)input;
		}
	}
	//actions
	void Input::pull_events()
	{
		//message
		MSG message;
		//send all windows messages
		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
	}

	//subscrive events
	void Input::subscrive_keyboard_listener(std::function<void(KeyboardEvent kevent, short mode, ActionEvent action)> listener)
	{
		auto* input = (Win32::InputWin32*)m_native;
		input->m_keyboard = listener;
	}
	
	void Input::subscrive_character_listener(std::function<void(int character, short mode, int plain)> listener)
	{
		auto* input = (Win32::InputWin32*)m_native;
		input->m_character = listener;
	}

	void Input::subscrive_mouse_button_listener(std::function<void(MouseButtonEvent mevent, ActionEvent action)> listener)
	{
		auto* input = (Win32::InputWin32*)m_native;
		input->m_mouse_button = listener;
	}

	void Input::subscrive_mouse_move_listener(std::function<void(double x, double y)> listener)
	{
		auto* input = (Win32::InputWin32*)m_native;
		input->m_mouse_move = listener;
	}
	
	void Input::subscrive_mouse_scroll_listener(std::function<void(double scroll)> listener)
	{
		auto* input = (Win32::InputWin32*)m_native;
		input->m_scroll = listener;
	}

	void Input::subscrive_touch_listener(std::function<void(FingerEvent fevent, ActionEvent action)> listener) 
	{
		auto* input = (Win32::InputWin32*)m_native;
		input->m_touch = listener;
	}

	void Input::subscrive_window_listener(std::function<void(WindowEvent wevent)> listener)
	{
		auto* input = (Win32::InputWin32*)m_native;
		input->m_window = listener;
	}

	void Input::subscrive_window_move_listener(std::function<void(double x, double y)> listener)
	{
		auto* input = (Win32::InputWin32*)m_native;
		input->m_window_move = listener;
	}
	////////////////////////////////////////////////////////////////////////////////////


}
}
