//
//  Square
//
//  Created by Gabriele on 14/08/16.
//  Copyright � 2016 Gabriele. All rights reserved.
//
#pragma once
#include "Square/Config.h"

namespace Square
{
namespace Video
{
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	SQUARE_API void init();
	SQUARE_API void close();
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	class SQUARE_API Screen
	{
	public:
		//static
		static unsigned int count();
		static Screen by_index(unsigned int index);
		//info
		void*  conteiner() const;
		//size
		void get_size(unsigned int& w, unsigned int& h) const;
		//window
		~Screen();
		//end
	protected:
		//window
		Screen();
		//data
		void* m_native;
	};
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	struct SQUARE_API ContextInfo
	{
		enum anti_aliasing
		{
			NOAA = 0,
			MSAAx2 = 2,
			MSAAx4 = 4,
			MSAAx8 = 8,
			MSAAx16 = 16,
			MSAAx32 = 32,
			MSAAx64 = 64,
			CSAA = 128,
			CSAAQ = 256,
			BESTAA = 512
		};
		enum context_type
		{
			CTX_OPENGL,
			CTX_OPENGL_ES,
			CTX_DIRECTX
		};
		context_type  m_type{ CTX_OPENGL };
		unsigned int  m_version[2]{ 3, 0 };
		unsigned int  m_depth     { 24   };
		unsigned int  m_color     { 24   };
		unsigned int  m_stencil   { 8    };
		anti_aliasing m_anti_aliasing{ NOAA };
		bool		  m_debug{ false };
		//add default constructor
		ContextInfo() {}
	};

	struct SQUARE_API WindowInfo
	{
		const Screen*   m_screen{ nullptr };
		ContextInfo	    m_context;
		std::string		m_title		{ "Square" };
		unsigned int	m_size[2]	{ 1280, 720 };
		int				m_position[2]{ 0,	0   };
		bool			m_fullscreen{ false };
		bool			m_resize    { false };

		WindowInfo() {}
		WindowInfo(Screen* screen) :m_screen(screen) {}
	};

	struct SQUARE_API DeviceResources
	{
	public:
		virtual void callback_target_changed(std::function<void(DeviceResources*)>) = 0;

		virtual unsigned int width() = 0;
		virtual unsigned int height() = 0;

		virtual void* get_device() = 0;
		virtual void* get_device_context(size_t i=0) = 0;
		virtual void* get_swap_chain() = 0;

		virtual void* get_render_target()  = 0;
		virtual void* get_depth_stencil_target() = 0;

		virtual void* get_render_resource() = 0;
		virtual void* get_depth_stencil_resource() = 0;
		//number of device_context
		virtual size_t number_of_device_context() = 0;
	};

	class SQUARE_API Window
	{
	public:
		//window
		Window(const WindowInfo& info);
		//window
		~Window();
		//native window
		void*			  native() const;
		void*			  conteiner() const;
		DeviceResources*  device() const;
		bool   valid() const;
		void   destoy();
		//window actions
		void clear();
		void swap();
		void acquire_context();
		//window info
		bool   is_fullscreen();
		bool   is_resizable();
		void   get_size(unsigned int& w, unsigned int& h);
		void   get_position(int& x, int& y);
		//set
		void   set_size(unsigned int w, unsigned int h);
		void   set_position(int x, int y);
		void   enable_resize(bool enable);
		void   enable_fullscreen(bool enable);

	protected:

		void*   m_native;
	};
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
}
}
