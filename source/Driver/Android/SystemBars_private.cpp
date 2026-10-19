//
//  Square
//
//  Android: the bars of the system (status, navigation) hidden over the game (immersive sticky:
//  a swipe from an edge shows them a little, over the frames), each time the activity gets its
//  focus back.
//
#include "SystemBars_private.h"
#include <android/api-level.h>
#include <jni.h>

namespace Square
{
namespace Video
{
namespace Android
{
	//WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE (API 30)
	static constexpr jint s_behavior_transient_by_swipe = 2;
	//View.SYSTEM_UI_FLAG_* (API 29): layout stable, hide navigation, fullscreen, immersive sticky
	static constexpr jint s_immersive_sticky_flags = 0x00000100  //LAYOUT_STABLE
	                                               | 0x00000200  //LAYOUT_HIDE_NAVIGATION
	                                               | 0x00000400  //LAYOUT_FULLSCREEN
	                                               | 0x00000002  //HIDE_NAVIGATION
	                                               | 0x00000004  //FULLSCREEN
	                                               | 0x00001000; //IMMERSIVE_STICKY

	//the callback of the focus of the glue, called before ours
	static void (*s_glue_focus)(ANativeActivity* activity, int focused) = nullptr;

	//a call failed (a method missing on this Android): its exception cleared, nothing more called
	static bool failed(JNIEnv* env)
	{
		if (!env->ExceptionCheck())
		{
			return false;
		}
		env->ExceptionClear();
		return true;
	}

	//a method of an object returning an object (null: none, or it failed)
	static jobject call_object(JNIEnv* env, jobject object, const char* name, const char* signature)
	{
		jclass object_class = env->GetObjectClass(object);
		jmethodID method = env->GetMethodID(object_class, name, signature);
		env->DeleteLocalRef(object_class);
		if (failed(env) || !method)
		{
			return nullptr;
		}
		jobject result = env->CallObjectMethod(object, method);
		if (failed(env))
		{
			return nullptr;
		}
		return result;
	}

	//a method of an object with an int argument (false: it failed)
	static bool call_void_int(JNIEnv* env, jobject object, const char* name, jint value)
	{
		jclass object_class = env->GetObjectClass(object);
		jmethodID method = env->GetMethodID(object_class, name, "(I)V");
		env->DeleteLocalRef(object_class);
		if (failed(env) || !method)
		{
			return false;
		}
		env->CallVoidMethod(object, method, value);
		return !failed(env);
	}

	//WindowInsets.Type.systemBars() (0: it failed)
	static jint system_bars_types(JNIEnv* env)
	{
		jclass type_class = env->FindClass("android/view/WindowInsets$Type");
		if (failed(env) || !type_class)
		{
			return 0;
		}
		jmethodID system_bars = env->GetStaticMethodID(type_class, "systemBars", "()I");
		jint types = 0;
		if (!failed(env) && system_bars)
		{
			types = env->CallStaticIntMethod(type_class, system_bars);
		}
		env->DeleteLocalRef(type_class);
		if (failed(env))
		{
			return 0;
		}
		return types;
	}

	//Android 11+: the controller of the insets of the window, its system bars hidden
	static void hide_with_controller(JNIEnv* env, jobject window)
	{
		jobject controller = call_object(env, window,
		                                 "getInsetsController",
		                                 "()Landroid/view/WindowInsetsController;");
		if (!controller)
		{
			return;
		}
		const jint types = system_bars_types(env);
		if (types && call_void_int(env, controller, "setSystemBarsBehavior", s_behavior_transient_by_swipe))
		{
			call_void_int(env, controller, "hide", types);
		}
		env->DeleteLocalRef(controller);
	}

	//Android 10: the flags of the decor view of the window
	static void hide_with_flags(JNIEnv* env, jobject window)
	{
		jobject decor = call_object(env, window, "getDecorView", "()Landroid/view/View;");
		if (!decor)
		{
			return;
		}
		call_void_int(env, decor, "setSystemUiVisibility", s_immersive_sticky_flags);
		env->DeleteLocalRef(decor);
	}

	void hide_system_bars(ANativeActivity* activity)
	{
		if (!activity || !activity->env || !activity->clazz)
		{
			return;
		}
		JNIEnv* env = activity->env;
		jobject window = call_object(env, activity->clazz, "getWindow", "()Landroid/view/Window;");
		if (!window)
		{
			return;
		}
		if (android_get_device_api_level() >= 30)
		{
			hide_with_controller(env, window);
		}
		else
		{
			hide_with_flags(env, window);
		}
		env->DeleteLocalRef(window);
	}

	static void focus_changed(ANativeActivity* activity, int focused)
	{
		if (s_glue_focus)
		{
			s_glue_focus(activity, focused);
		}
		if (focused)
		{
			hide_system_bars(activity);
		}
	}

	void keep_system_bars_hidden(ANativeActivity* activity)
	{
		if (!activity || !activity->callbacks)
		{
			return;
		}
		//(once: the callback of the glue wrapped by ours)
		if (activity->callbacks->onWindowFocusChanged == focus_changed)
		{
			return;
		}
		s_glue_focus = activity->callbacks->onWindowFocusChanged;
		activity->callbacks->onWindowFocusChanged = focus_changed;
	}
}
}
}
