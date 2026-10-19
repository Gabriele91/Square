//
//  Square
//
//  Android: the NativeActivity (the glue of the NDK: android_main runs the game in its own
//  thread), its life (its window comes and goes), its events, and the screens, the windows and
//  the inputs of the engine over it.
//
#include "Wrapper_private.h"
#include "Screen_private.h"
#include "Window_private.h"
#include "Input_private.h"
#include "SystemBars_private.h"
#include "Square/Driver/Android.h"
#include <android/log.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <algorithm>
#include <sstream>

namespace Square
{
namespace Video
{
namespace Android
{
	////////////////////////////////////////////////////////////////////////////////////
	ActivityState  s_activity;
	WrapperContext s_os_context;

	////////////////////////////////////////////////////////////////////////////////////
	static InputAndroid* input_create(WindowAndroid* wnd, Input* input);
	static void input_delete(InputAndroid*& in);
	static WindowAndroid* window_create(const WindowInfo& info, Window* window);
	static void	window_delete(WindowAndroid*& window);

	////////////////////////////////////////////////////////////////////////////////////
	void native_window_size(unsigned int size[2])
	{
		size[0] = 0;
		size[1] = 0;
		if (s_activity.m_native_window)
		{
			size[0] = (unsigned int)std::max(0, ANativeWindow_getWidth(s_activity.m_native_window));
			size[1] = (unsigned int)std::max(0, ANativeWindow_getHeight(s_activity.m_native_window));
		}
	}

	WindowAndroid* main_window()
	{
		if (s_os_context.m_windows.empty())
		{
			return nullptr;
		}
		return s_os_context.m_windows.begin()->second;
	}

	InputAndroid* main_input()
	{
		if (s_os_context.m_input.empty())
		{
			return nullptr;
		}
		return s_os_context.m_input.begin()->second;
	}

	static void send_window_event(WindowEvent event)
	{
		if (InputAndroid* input = main_input())
		{
			input->send_window_event(event);
		}
	}

	////////////////////////////////////////////////////////////////////////////////////
	// TILT
	//rotation of the display, 0..3 (JNI: Display.getRotation)
	static int display_rotation()
	{
		android_app* app = s_activity.m_app;
		if (!app || !app->activity || !app->activity->vm)
		{
			return 0;
		}
		JavaVM* vm = app->activity->vm;
		JNIEnv* env = nullptr;
		if (vm->AttachCurrentThread(&env, nullptr) != JNI_OK || !env)
		{
			return s_activity.m_rotation;
		}
		int rotation = s_activity.m_rotation;
		jobject activity = app->activity->clazz;
		jclass activity_class = env->GetObjectClass(activity);
		jmethodID get_manager = env->GetMethodID(activity_class,
		                                         "getWindowManager",
		                                         "()Landroid/view/WindowManager;");
		jobject manager = nullptr;
		if (get_manager)
		{
			manager = env->CallObjectMethod(activity, get_manager);
		}
		if (manager)
		{
			jclass manager_class = env->GetObjectClass(manager);
			jmethodID get_display = env->GetMethodID(manager_class,
			                                         "getDefaultDisplay",
			                                         "()Landroid/view/Display;");
			jobject display = nullptr;
			if (get_display)
			{
				display = env->CallObjectMethod(manager, get_display);
			}
			if (display)
			{
				jclass display_class = env->GetObjectClass(display);
				jmethodID get_rotation = env->GetMethodID(display_class, "getRotation", "()I");
				if (get_rotation)
				{
					rotation = int(env->CallIntMethod(display, get_rotation));
				}
				env->DeleteLocalRef(display_class);
				env->DeleteLocalRef(display);
			}
			env->DeleteLocalRef(manager_class);
			env->DeleteLocalRef(manager);
		}
		env->DeleteLocalRef(activity_class);
		if (env->ExceptionCheck())
		{
			env->ExceptionClear();
		}
		vm->DetachCurrentThread();
		return rotation & 3;
	}

	//sensor on while asked and in the foreground
	static void tilt_update()
	{
		const bool on = s_activity.m_tilt_wanted && s_activity.m_resumed && s_activity.m_app;
		if (on == s_activity.m_tilt_on)
		{
			return;
		}
		if (on && !s_activity.m_sensor_queue)
		{
			s_activity.m_sensors = ASensorManager_getInstanceForPackage(nullptr);
			if (!s_activity.m_sensors)
			{
				return;
			}
			s_activity.m_gravity = ASensorManager_getDefaultSensor(s_activity.m_sensors, ASENSOR_TYPE_GRAVITY);
			if (!s_activity.m_gravity)
			{
				s_activity.m_gravity = ASensorManager_getDefaultSensor(s_activity.m_sensors, ASENSOR_TYPE_ACCELEROMETER);
			}
			if (!s_activity.m_gravity)
			{
				__android_log_print(ANDROID_LOG_WARN, "Square", "Android: no gravity sensor, no tilt");
				s_activity.m_tilt_wanted = false;
				return;
			}
			s_activity.m_sensor_queue = ASensorManager_createEventQueue(s_activity.m_sensors,
		                                                            s_activity.m_app->looper,
		                                                            LOOPER_ID_USER,
		                                                            nullptr,
		                                                            nullptr);
			s_activity.m_rotation = display_rotation();
		}
		if (!s_activity.m_sensor_queue)
		{
			return;
		}
		if (on)
		{
			//60 a second
			ASensorEventQueue_enableSensor(s_activity.m_sensor_queue, s_activity.m_gravity);
			ASensorEventQueue_setEventRate(s_activity.m_sensor_queue, s_activity.m_gravity, 16667);
		}
		else
		{
			ASensorEventQueue_disableSensor(s_activity.m_sensor_queue, s_activity.m_gravity);
		}
		s_activity.m_tilt_on = on;
	}

	//last gravity of the sensor, on the screen (its rotation)
	static void tilt_events()
	{
		if (!s_activity.m_sensor_queue)
		{
			return;
		}
		ASensorEvent event;
		bool any = false;
		float device[3]{ 0.0f, 0.0f, 0.0f };
		while (ASensorEventQueue_getEvents(s_activity.m_sensor_queue, &event, 1) > 0)
		{
			device[0] = event.vector.x;
			device[1] = event.vector.y;
			device[2] = event.vector.z;
			any = true;
		}
		InputAndroid* input = main_input();
		if (!any || !input)
		{
			return;
		}
		float x = device[0], y = device[1];
		switch (s_activity.m_rotation)
		{
		case 1:  x = -device[1]; y = device[0];  break;
		case 2:  x = -device[0]; y = -device[1]; break;
		case 3:  x = device[1];  y = -device[0]; break;
		default: break;
		}
		input->send_tilt_event(x, y, device[2]);
	}

	void enable_tilt(bool enable)
	{
		s_activity.m_tilt_wanted = enable;
		tilt_update();
	}

	////////////////////////////////////////////////////////////////////////////////////
	// ACTIVITY
	//the window of the activity: made (at the start, back from the background), gone
	static void window_initialized(android_app* app)
	{
		const bool again = s_activity.m_window_lost;
		s_activity.m_native_window = app->window;
		s_activity.m_window_lost = false;
		if (s_activity.m_sensor_queue)
		{
			s_activity.m_rotation = display_rotation();
		}
		if (WindowAndroid* window = main_window())
		{
			window->update_size();
			//(its frame rate on the new native window)
			if (window->m_vk_device)
			{
				window->m_vk_device->apply_frame_rate();
			}
		}
		if (again)
		{
			send_window_event(WindowEvent::RESIZE);
		}
	}

	static void window_terminated()
	{
		s_activity.m_native_window = nullptr;
		s_activity.m_window_lost = true;
		send_window_event(WindowEvent::MINIMIZED);
	}

	static void window_resized()
	{
		WindowAndroid* window = main_window();
		if (!window)
		{
			return;
		}
		if (window->update_size())
		{
			send_window_event(WindowEvent::RESIZE);
		}
	}

	static void activity_command(android_app* app, int32_t command)
	{
		switch (command)
		{
		case APP_CMD_INIT_WINDOW:
			window_initialized(app);
			break;
		case APP_CMD_TERM_WINDOW:
			window_terminated();
			break;
		case APP_CMD_CONFIG_CHANGED:
			//(turned: the other landscape, the tilt turned with it)
			if (s_activity.m_sensor_queue)
			{
				s_activity.m_rotation = display_rotation();
			}
			window_resized();
			break;
		case APP_CMD_WINDOW_RESIZED:
		case APP_CMD_CONTENT_RECT_CHANGED:
			window_resized();
			break;
		case APP_CMD_GAINED_FOCUS:
			s_activity.m_focused = true;
			send_window_event(WindowEvent::GET_FOCUS);
			break;
		case APP_CMD_LOST_FOCUS:
			s_activity.m_focused = false;
			send_window_event(WindowEvent::LOST_FOCUS);
			break;
		case APP_CMD_RESUME:
			s_activity.m_resumed = true;
			tilt_update();
			break;
		case APP_CMD_PAUSE:
			s_activity.m_resumed = false;
			tilt_update();
			break;
		default:
			break;
		}
	}

	static int32_t activity_input(android_app* app, AInputEvent* event)
	{
		InputAndroid* input = main_input();
		if (!input)
		{
			return 0;
		}
		return input_event(*input, event);
	}

	//the game can draw: the activity in the foreground with its window
	static bool activity_running()
	{
		return s_activity.m_native_window && s_activity.m_resumed;
	}

	//the events of the activity: all of them, waited while the game can not draw (in the
	//background: no frame, no battery); its end once (the window closed)
	static void activity_events()
	{
		android_app* app = s_activity.m_app;
		if (!app)
		{
			return;
		}
		//the size of the frames asked by the game (set_size): its event with the others
		if (s_activity.m_resize_pending)
		{
			s_activity.m_resize_pending = false;
			send_window_event(WindowEvent::RESIZE);
		}
		while (true)
		{
			//in the background: wait for an event (-1), else only the ones there (0)
			int timeout = 0;
			if (!activity_running() && !app->destroyRequested)
			{
				timeout = -1;
			}
			android_poll_source* source = nullptr;
			int events = 0;
			const int ident = ALooper_pollOnce(timeout, nullptr, &events, (void**)&source);
			if (ident >= 0 && source)
			{
				source->process(app, source);
			}
			//(the tilt sensor: its queue, no source of the glue)
			if (ident == LOOPER_ID_USER)
			{
				tilt_events();
			}
			if (app->destroyRequested)
			{
				if (!s_activity.m_close_sent)
				{
					s_activity.m_close_sent = true;
					send_window_event(WindowEvent::CLOSE);
				}
				return;
			}
			if (ident == ALOOPER_POLL_TIMEOUT || ident == ALOOPER_POLL_ERROR)
			{
				if (activity_running())
				{
					return;
				}
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////////////
	// START, ARGUMENTS, END
	//the folders: the working one (the resources of the game, pushed by adb), home (the files
	//of the application: the settings of the game)
	static void activity_folders(android_app* app)
	{
		const char* resources = app->activity->externalDataPath;
		const char* home = app->activity->internalDataPath;
		if (resources && chdir(resources) != 0)
		{
			__android_log_print(ANDROID_LOG_ERROR, "Square", "Android: no resource folder %s", resources);
		}
		//(also the data of the application, app_data_dir: the shader cache; no .local/share there)
		if (home)
		{
			setenv("HOME", home, 1);
			setenv("XDG_DATA_HOME", home, 1);
		}
	}

	//the arguments of the file args.txt in the resource folder (the working one)
	static void file_arguments(Vector<String>& arguments)
	{
		std::ifstream file("args.txt");
		if (!file.is_open())
		{
			return;
		}
		String argument;
		while (file >> argument)
		{
			arguments.push_back(argument);
		}
	}

	//the activity in the foreground, with its window and its focus: its orientation already the
	//one of the game (landscape), the swap chain made once at its size
	static bool activity_ready()
	{
		return s_activity.m_native_window && s_activity.m_resumed && s_activity.m_focused;
	}

	//the environment of the file env.txt in the resource folder (a line NAME=VALUE): the variables
	//of the engine and of the game (SQUARE_HEAP, RUSH_BENCH...), no shell to set them on Android
	//a line of env.txt as a variable (NAME=VALUE; an empty line, a #comment, no name: none)
	static void line_environment(String line)
	{
		if (!line.empty() && line.back() == '\r')
		{
			line.pop_back();
		}
		if (line.empty() || line[0] == '#')
		{
			return;
		}
		const size_t equal = line.find('=');
		if (equal == String::npos || equal == 0)
		{
			return;
		}
		const String name = line.substr(0, equal);
		const String value = line.substr(equal + 1);
		setenv(name.c_str(), value.c_str(), 1);
		__android_log_print(ANDROID_LOG_INFO, "Square", "Android: %s=%s (env.txt)", name.c_str(), value.c_str());
	}

	static void file_environment()
	{
		std::ifstream file("env.txt");
		if (!file.is_open())
		{
			return;
		}
		String line;
		while (std::getline(file, line))
		{
			line_environment(line);
		}
	}

	static void wait_window()
	{
		while (!activity_ready() && !s_activity.m_app->destroyRequested)
		{
			android_poll_source* source = nullptr;
			int events = 0;
			if (ALooper_pollOnce(-1, nullptr, &events, (void**)&source) >= 0 && source)
			{
				source->process(s_activity.m_app, source);
			}
		}
	}

	void attach(android_app* app)
	{
		s_activity.m_app = app;
		app->userData = nullptr;
		app->onAppCmd = activity_command;
		app->onInputEvent = activity_input;
		//the bars of the system hidden at each focus (before the first one: the activity is
		//still starting)
		keep_system_bars_hidden(app->activity);
		activity_folders(app);
		file_environment();
		//the game starts with its window (the size of the screen)
		wait_window();
	}

	void arguments(int& argc, const char**& argv)
	{
		s_activity.m_arguments.clear();
		s_activity.m_arguments.push_back("Square");
		s_activity.m_arguments.push_back("-b");
		s_activity.m_arguments.push_back("vk");
		file_arguments(s_activity.m_arguments);
		s_activity.m_argv.clear();
		for (const String& argument : s_activity.m_arguments)
		{
			s_activity.m_argv.push_back(argument.c_str());
		}
		argc = int(s_activity.m_argv.size());
		argv = s_activity.m_argv.data();
	}

	void detach()
	{
		android_app* app = s_activity.m_app;
		if (app && !app->destroyRequested)
		{
			ANativeActivity_finish(app->activity);
			while (!app->destroyRequested)
			{
				android_poll_source* source = nullptr;
				int events = 0;
				if (ALooper_pollOnce(-1, nullptr, &events, (void**)&source) >= 0 && source)
				{
					source->process(app, source);
				}
			}
		}
		//the process ends with the activity: the next one starts the engine from zero
		std::exit(0);
	}

	android_app* application()
	{
		return s_activity.m_app;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// WINDOW, INPUT
	static WindowAndroid* window_create(const WindowInfo& info, Window* window)
	{
		WindowAndroid* wnd = new WindowAndroid(info);
		wnd->m_window_ref = window;
		s_os_context.m_windows[wnd] = wnd;
		return wnd;
	}

	static void window_delete(WindowAndroid*& window)
	{
		auto it_wnd = s_os_context.m_windows.find(window);
		if (it_wnd != s_os_context.m_windows.end())
		{
			s_os_context.m_windows.erase(it_wnd);
		}
		if (window->m_input_ref)
		{
			window->m_input_ref->m_window_ref = nullptr;
		}
		delete window;
		window = nullptr;
	}

	static InputAndroid* input_create(WindowAndroid* wnd, Input* input)
	{
		InputAndroid* in = new InputAndroid();
		in->m_window_ref = wnd;
		wnd->m_input_ref = in;
		in->m_input_ref = input;
		s_os_context.m_input[in] = in;
		return in;
	}

	static void input_delete(InputAndroid*& in)
	{
		auto it_in = s_os_context.m_input.find(in);
		if (it_in != s_os_context.m_input.end())
		{
			s_os_context.m_input.erase(it_in);
		}
		if (in->m_window_ref && in->m_window_ref->m_input_ref)
		{
			in->m_window_ref->m_input_ref = nullptr;
		}
		delete in;
		in = nullptr;
	}
}

	////////////////////////////////////////////////////////////////////////////////////
	// VIDEO
	void init()
	{
		//one screen: the display of the activity
		Android::s_os_context.m_screens.clear();
		Android::s_os_context.m_screens.emplace_back(0);
		Android::s_os_context.m_mouse_pointer = -1;
	}

	void close()
	{
		//remove all inputs/windows
		if (!Android::s_os_context.m_input.empty())
		{
			for (auto inp_it : Android::s_os_context.m_input)
			{
				inp_it.second->m_input_ref->destoy();
			}
		}
		if (!Android::s_os_context.m_windows.empty())
		{
			for (auto wnd_it : Android::s_os_context.m_windows)
			{
				wnd_it.second->m_window_ref->destoy();
			}
		}
		Android::s_os_context.m_input.clear();
		Android::s_os_context.m_windows.clear();
		Android::s_os_context.m_screens.clear();
	}

	////////////////////////////////////////////////////////////////////////////////////
	//	SCREEN
	unsigned int Screen::count()
	{
		return (unsigned int)Android::s_os_context.m_screens.size();
	}

	Screen	Screen::by_index(unsigned int index)
	{
		Screen output;
		output.m_native = &Android::s_os_context.m_screens[index];
		return output;
	}

	Screen::Screen() { /* void */ }

	Screen::~Screen() { /* void */ }

	void* Screen::conteiner() const
	{
		return m_native;
	}

	void Screen::get_size(unsigned int& width, unsigned int& height) const
	{
		auto* screen = (Android::ScreenAndroid*)m_native;
		unsigned int size[2];
		screen->get_size(size);
		width = size[0];
		height = size[1];
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// WINDOW
	Window::Window(const WindowInfo& info)
	{
		m_native = Android::window_create(info, this);
	}

	Window::~Window()
	{
		destoy();
	}

	void* Window::native() const
	{
		return (void*)Android::s_activity.m_native_window;
	}

	void* Window::conteiner() const
	{
		return m_native;
	}

	DeviceResources* Window::device() const
	{
		const auto* window = (const Android::WindowAndroid*)m_native;
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
			auto* window = (Android::WindowAndroid*)m_native;
			Android::window_delete(window);
			m_native = (void*)window;
		}
	}

	void Window::clear()
	{
		//none
	}

	void Window::swap()
	{
		auto* window = (Android::WindowAndroid*)m_native;
		window->swap();
	}

	void Window::acquire_context()
	{
		auto* window = (Android::WindowAndroid*)m_native;
		window->acquire_context();
	}

	bool Window::is_fullscreen()
	{
		auto* window = (Android::WindowAndroid*)m_native;
		return window->is_fullscreen();
	}

	bool Window::is_resizable()
	{
		auto* window = (Android::WindowAndroid*)m_native;
		return window->is_resizable();
	}

	void Window::get_size(unsigned int& w, unsigned int& h)
	{
		auto* window = (Android::WindowAndroid*)m_native;
		unsigned int size[2];
		window->get_size(size);
		w = size[0];
		h = size[1];
	}

	void Window::get_position(int& x, int& y)
	{
		auto* window = (Android::WindowAndroid*)m_native;
		int position[2];
		window->get_position(position);
		x = position[0];
		y = position[1];
	}

	void Window::set_size(unsigned int w, unsigned int h)
	{
		auto* window = (Android::WindowAndroid*)m_native;
		unsigned int size[2]{ w, h };
		window->set_size(size);
	}

	void Window::set_position(int x, int y)
	{
		auto* window = (Android::WindowAndroid*)m_native;
		int position[2]{ x, y };
		window->set_position(position);
	}

	void Window::enable_resize(bool enable)
	{
		auto* window = (Android::WindowAndroid*)m_native;
		window->enable_resize(enable);
	}

	void Window::enable_fullscreen(bool enable)
	{
		auto* window = (Android::WindowAndroid*)m_native;
		window->enable_fullscreen(enable);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// INPUT
	Input::Input(Window* wnd)
	{
		m_native = Android::input_create((Android::WindowAndroid*)wnd->conteiner(), this);
	}

	Input::~Input()
	{
		destoy();
	}

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
			auto* input = (Android::InputAndroid*)m_native;
			Android::input_delete(input);
			m_native = (void*)input;
		}
	}

	void Input::pull_events()
	{
		Android::activity_events();
	}

	void Input::subscrive_keyboard_listener(std::function<void(KeyboardEvent kevent, short mode, ActionEvent action)> listener)
	{
		auto* input = (Android::InputAndroid*)m_native;
		input->m_keyboard = listener;
	}

	void Input::subscrive_character_listener(std::function<void(int character, short mode, int plain)> listener)
	{
		auto* input = (Android::InputAndroid*)m_native;
		input->m_character = listener;
	}

	void Input::subscrive_mouse_button_listener(std::function<void(MouseButtonEvent mevent, ActionEvent action)> listener)
	{
		auto* input = (Android::InputAndroid*)m_native;
		input->m_mouse_button = listener;
	}

	void Input::subscrive_mouse_move_listener(std::function<void(double x, double y)> listener)
	{
		auto* input = (Android::InputAndroid*)m_native;
		input->m_mouse_move = listener;
	}

	void Input::subscrive_mouse_scroll_listener(std::function<void(double scroll)> listener)
	{
		auto* input = (Android::InputAndroid*)m_native;
		input->m_scroll = listener;
	}

	void Input::subscrive_touch_listener(std::function<void(FingerEvent fevent, ActionEvent action)> listener)
	{
		auto* input = (Android::InputAndroid*)m_native;
		input->m_touch = listener;
	}

	void Input::subscrive_finger_listener(std::function<void(FingerEvent finger, ActionEvent action, double x, double y)> listener)
	{
		auto* input = (Android::InputAndroid*)m_native;
		input->m_finger = listener;
	}

	void Input::subscrive_tilt_listener(std::function<void(double x, double y, double z)> listener)
	{
		auto* input = (Android::InputAndroid*)m_native;
		input->m_tilt = listener;
	}

	void Input::enable_tilt(bool enable)
	{
		Android::enable_tilt(enable);
	}

	void Input::subscrive_window_listener(std::function<void(WindowEvent wevent)> listener)
	{
		auto* input = (Android::InputAndroid*)m_native;
		input->m_window = listener;
	}

	void Input::subscrive_window_move_listener(std::function<void(double x, double y)> listener)
	{
		auto* input = (Android::InputAndroid*)m_native;
		input->m_window_move = listener;
	}
	////////////////////////////////////////////////////////////////////////////////////
}
}
