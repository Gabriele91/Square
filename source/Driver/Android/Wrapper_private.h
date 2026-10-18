//
//  Square
//
//  Android: the state of the NativeActivity (the application of the glue of the NDK, its window,
//  its life) and of the windows and inputs of the engine over it.
//
#pragma once
#include "Square/Config.h"
#include "Square/Driver/Window.h"
#include "Square/Driver/Input.h"

///////////////////////////////////////////////////
// Android
#include <android/native_window.h>
#include <android/input.h>
#include <android/sensor.h>
#include <android_native_app_glue.h>

namespace Square
{
namespace Video
{
namespace Android
{
	//struct declaretion
	struct ScreenAndroid;
	struct WindowAndroid;
	struct InputAndroid;

	//the activity: set by attach (before the engine), changed by its commands
	struct ActivityState
	{
		android_app*   m_app{ nullptr };
		ANativeWindow* m_native_window{ nullptr }; //null while the activity is in the background
		bool           m_resumed{ false };
		bool           m_focused{ false };
		bool           m_close_sent{ false };
		bool           m_window_lost{ false };     //the window went: the next one is a new one
		bool           m_resize_pending{ false };  //the size of the frames changed (set_size): RESIZE
		Vector<String> m_arguments;
		Vector<const char*> m_argv;
		//the tilt: the gravity sensor (else the accelerometer), its queue on the looper of the game,
		//on while the game asks it (enable_tilt) and the activity is in the foreground; the rotation
		//of the display (0, 1, 2, 3: 0, 90, 180, 270 degrees; the gravity in the space of the screen)
		ASensorManager*    m_sensors{ nullptr };
		const ASensor*     m_gravity{ nullptr };
		ASensorEventQueue* m_sensor_queue{ nullptr };
		bool               m_tilt_wanted{ false };
		bool               m_tilt_on{ false };
		int                m_rotation{ 0 };
	};
	extern ActivityState s_activity;

	// Context: the screens, the windows and the inputs of the engine
	struct WrapperContext
	{
		Vector<ScreenAndroid>                        m_screens;
		UnorderedMap<WindowAndroid*, WindowAndroid*> m_windows;
		UnorderedMap<InputAndroid*, InputAndroid*>   m_input;
		//the pointer of the touch screen that is the mouse (the first finger down), -1 none
		int32_t m_mouse_pointer{ -1 };
	};
	extern WrapperContext s_os_context;

	//the size of the native window (0, 0 without it)
	void native_window_size(unsigned int size[2]);
	//the window of the engine (one on Android), null before it is made
	WindowAndroid* main_window();
	//the input of the engine (of its window), null before it is made
	InputAndroid* main_input();
	//the tilt sensor asked on or off (on: while the activity is in the foreground)
	void enable_tilt(bool enable);
}
}
}
