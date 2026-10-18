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
	static constexpr jint s_immersive_sticky_flags = 0x00000100 | 0x00000200 | 0x00000400 | 0x00000002 | 0x00000004 | 0x00001000;

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

	//the callback of the focus of the glue, called before ours
	static void (*s_glue_focus)(ANativeActivity* activity, int focused) = nullptr;

	//Android 11+: the controller of the insets of the window, its system bars hidden
	static void hide_with_controller(JNIEnv* env, jobject window)
	{
		jclass window_class = env->GetObjectClass(window);
		jmethodID get_controller = env->GetMethodID(window_class, "getInsetsController", "()Landroid/view/WindowInsetsController;");
		jobject controller = failed(env) ? nullptr : env->CallObjectMethod(window, get_controller);
		jclass type_class = (failed(env) || !controller) ? nullptr : env->FindClass("android/view/WindowInsets$Type");
		if (type_class && !failed(env))
		{
			jmethodID system_bars = env->GetStaticMethodID(type_class, "systemBars", "()I");
			const jint types = failed(env) ? 0 : env->CallStaticIntMethod(type_class, system_bars);
			jclass controller_class = env->GetObjectClass(controller);
			jmethodID behavior = env->GetMethodID(controller_class, "setSystemBarsBehavior", "(I)V");
			jmethodID hide = failed(env) ? nullptr : env->GetMethodID(controller_class, "hide", "(I)V");
			if (types && behavior && hide && !failed(env))
			{
				env->CallVoidMethod(controller, behavior, s_behavior_transient_by_swipe);
				env->CallVoidMethod(controller, hide, types);
			}
			env->DeleteLocalRef(controller_class);
			env->DeleteLocalRef(type_class);
		}
		if (controller)
		{
			env->DeleteLocalRef(controller);
		}
		env->DeleteLocalRef(window_class);
	}

	//Android 10: the flags of the decor view of the window
	static void hide_with_flags(JNIEnv* env, jobject window)
	{
		jclass window_class = env->GetObjectClass(window);
		jmethodID get_decor = env->GetMethodID(window_class, "getDecorView", "()Landroid/view/View;");
		jobject decor = failed(env) ? nullptr : env->CallObjectMethod(window, get_decor);
		if (decor && !failed(env))
		{
			jclass view_class = env->GetObjectClass(decor);
			jmethodID set_flags = env->GetMethodID(view_class, "setSystemUiVisibility", "(I)V");
			if (!failed(env))
			{
				env->CallVoidMethod(decor, set_flags, s_immersive_sticky_flags);
			}
			env->DeleteLocalRef(view_class);
		}
		if (decor)
		{
			env->DeleteLocalRef(decor);
		}
		env->DeleteLocalRef(window_class);
	}

	void hide_system_bars(ANativeActivity* activity)
	{
		JNIEnv* env = activity ? activity->env : nullptr;
		if (!env || !activity->clazz)
		{
			return;
		}
		jclass activity_class = env->GetObjectClass(activity->clazz);
		jmethodID get_window = env->GetMethodID(activity_class, "getWindow", "()Landroid/view/Window;");
		jobject window = failed(env) ? nullptr : env->CallObjectMethod(activity->clazz, get_window);
		if (window && !failed(env))
		{
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
		env->DeleteLocalRef(activity_class);
		//(no exception left to the activity)
		failed(env);
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
		if (!activity || !activity->callbacks || activity->callbacks->onWindowFocusChanged == focus_changed)
		{
			return;
		}
		s_glue_focus = activity->callbacks->onWindowFocusChanged;
		activity->callbacks->onWindowFocusChanged = focus_changed;
	}
}
}
}
