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

#include <d3d11_3.h>
#include <DXGI1_3.h>
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"Dxgi.lib")

namespace Square
{
namespace Video
{
namespace Win32
{
	///////////////////////////////////////////////////////////////////////////////////
	struct SQUARE_API DeviceResourcesDX : public DeviceResources
	{
	public:
		DeviceResourcesDX(const WindowInfo& m_info);
		//build
		HRESULT create_device_resources(HWND hWnd);
		//present
		void present();
		HRESULT configure_backbuffer();
		HRESULT release_backbuffer();
		HRESULT go_fullscreen(UINT width = 0, UINT height = 0);
		HRESULT go_windowed(UINT width = 0, UINT height = 0);

		//implement
		virtual unsigned int width() override { return m_bb_desc.Width; }
		virtual unsigned int height() override { return m_bb_desc.Height; }

		virtual void* get_device()					   override { return (void*)m_d3d_device; }
		virtual void* get_device_context(size_t i = 0) override { return (void*)m_d3d_device_context; }
		virtual void* get_swap_chain()				   override { return (void*)m_DXGI_swap_chain; }

		virtual void* get_render_target()        override { return (void*)m_render_target; }
		virtual void* get_depth_stencil_target() override { return (void*)m_depth_stencil_view; }

		virtual void* get_render_resource()        override { return (void*)m_back_buffer; }
		virtual void* get_depth_stencil_resource() override { return (void*)m_depth_stencil; }

		virtual size_t number_of_device_context()  override { return 1; }

		virtual ~DeviceResourcesDX();

	protected:
		//-----------------------------------------------------------------------------
		// Direct3D device
		//-----------------------------------------------------------------------------
		ID3D11Device*        m_d3d_device;
		ID3D11DeviceContext* m_d3d_device_context; // immediate context
		IDXGISwapChain*      m_DXGI_swap_chain;


		//-----------------------------------------------------------------------------
		// DXGI swap chain device resources
		//-----------------------------------------------------------------------------
		ID3D11Texture2D*        m_back_buffer;
		ID3D11RenderTargetView* m_render_target;


		//-----------------------------------------------------------------------------
		// Direct3D device resources for the depth stencil
		//-----------------------------------------------------------------------------
		ID3D11Texture2D*         m_depth_stencil;
		ID3D11DepthStencilView*  m_depth_stencil_view;


		//-----------------------------------------------------------------------------
		// Direct3D device metadata and device resource metadata
		//-----------------------------------------------------------------------------
		D3D_FEATURE_LEVEL       m_feature_level;
		D3D11_TEXTURE2D_DESC    m_bb_desc;
		D3D11_VIEWPORT          m_viewport;
		const WindowInfo&		m_info;
	};

	DeviceResourcesDX::DeviceResourcesDX(const WindowInfo& info)
	: m_info(info)
	{
		m_d3d_device         = nullptr;
		m_d3d_device_context = nullptr;
		m_DXGI_swap_chain    = nullptr;

		m_back_buffer   = nullptr;
		m_render_target = nullptr;

		m_depth_stencil = nullptr;
		m_depth_stencil_view = nullptr;
	}
	
	DeviceResourcesDX::~DeviceResourcesDX()
	{
		if (m_render_target)      m_render_target->Release();
		if (m_back_buffer)        m_back_buffer->Release();
		if (m_depth_stencil_view) m_depth_stencil_view->Release();
		if (m_depth_stencil)      m_depth_stencil->Release();

		if (m_d3d_device_context) m_d3d_device_context->Release();
		if (m_DXGI_swap_chain)    m_DXGI_swap_chain->Release();
		if (m_d3d_device)         m_d3d_device->Release();
	}

	HRESULT DeviceResourcesDX::create_device_resources(HWND hWnd)
	{
		HRESULT hr = S_OK;

		//driver version
		D3D_FEATURE_LEVEL levels[] = 
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1,
		};

		D3D_DRIVER_TYPE  driver_types[]
		{
			D3D_DRIVER_TYPE_HARDWARE,
			D3D_DRIVER_TYPE_WARP,
			D3D_DRIVER_TYPE_REFERENCE,
		};

		//device
		UINT device_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
		
		//enable debug
		if(m_info.m_context.m_debug) device_flags |= D3D11_CREATE_DEVICE_DEBUG;

		//color format
		DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM;

		if (m_info.m_context.m_color >= 32)
			format = DXGI_FORMAT_B8G8R8A8_UNORM;
		else if (m_info.m_context.m_color >= 24)
			format = DXGI_FORMAT_B8G8R8X8_TYPELESS;
		else if (m_info.m_context.m_color <= 16)
			format = DXGI_FORMAT_B5G6R5_UNORM;

		//driver/swap chain info
		DXGI_SWAP_CHAIN_DESC desc;
		ZeroMemory(&desc, sizeof(DXGI_SWAP_CHAIN_DESC));
		desc.Windowed           = TRUE;
		desc.BufferCount        = 2;
		desc.BufferDesc.Width   = m_info.m_size[0];
		desc.BufferDesc.Height  = m_info.m_size[1];
		desc.BufferDesc.Format  = format;
		desc.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.SampleDesc.Count   = 1;    //multisampling setting
		desc.SampleDesc.Quality = 0;    //vendor-specific flag
		desc.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		desc.OutputWindow       = hWnd;

		//build		
		for (auto driver_type : driver_types)
		{
			hr = D3D11CreateDeviceAndSwapChain
			(
				nullptr,
				driver_type,
				nullptr,
				device_flags,
				levels,
				ARRAYSIZE(levels),
				D3D11_SDK_VERSION,
				&desc,
				&m_DXGI_swap_chain,
				&m_d3d_device,
				&m_feature_level,
				&m_d3d_device_context
			);
			if (SUCCEEDED(hr))
				break;
		}
		//set viewport
		ZeroMemory(&m_viewport, sizeof(D3D11_VIEWPORT));
		m_viewport.Height   = (float)desc.BufferDesc.Height;
		m_viewport.Width    = (float)desc.BufferDesc.Width;
		m_viewport.MinDepth = 0;
		m_viewport.MaxDepth = 1;		
		m_d3d_device_context->RSSetViewports( 1, &m_viewport );
		//ok
		return hr;
	}
	
	void DeviceResourcesDX::present()
	{
		if (m_DXGI_swap_chain)
		{
			m_DXGI_swap_chain->Present(1, 0);
		}
	}

	HRESULT DeviceResourcesDX::configure_backbuffer()
	{
		HRESULT hr = S_OK;

		//build back ground
		hr = m_DXGI_swap_chain->GetBuffer( 0, __uuidof(ID3D11Texture2D), (void**)&m_back_buffer);
		hr = m_d3d_device->CreateRenderTargetView(m_back_buffer, nullptr, &m_render_target );
		m_back_buffer->GetDesc(&m_bb_desc);

		//buffer type
		DXGI_FORMAT depth_stencil_type = DXGI_FORMAT_D32_FLOAT;

		if (m_info.m_context.m_depth == 32 && m_info.m_context.m_stencil)
			depth_stencil_type = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		if (m_info.m_context.m_depth == 24)
			depth_stencil_type = DXGI_FORMAT_D24_UNORM_S8_UINT;
		if (m_info.m_context.m_depth == 16)
			depth_stencil_type = DXGI_FORMAT_D16_UNORM;

		// Create a depth-stencil view for use with 3D rendering if needed.
		CD3D11_TEXTURE2D_DESC depth_stencil_desc
		(
			depth_stencil_type,
			static_cast<UINT> (m_bb_desc.Width),
			static_cast<UINT> (m_bb_desc.Height),
			1, // This depth stencil view has only one texture.
			1, // Use a single mipmap level.
			D3D11_BIND_DEPTH_STENCIL
		);
		m_d3d_device->CreateTexture2D( &depth_stencil_desc, nullptr, &m_depth_stencil);

		//View target
		CD3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc(D3D11_DSV_DIMENSION_TEXTURE2D);
		//D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc;
		//depth_stencil_view_desc.Format			 = depth_stencil_type;
		//depth_stencil_view_desc.ViewDimension	     = D3D11_DSV_DIMENSION_TEXTURE2D;
		//depth_stencil_view_desc.Texture2D.MipSlice = 1;
		m_d3d_device->CreateDepthStencilView(m_depth_stencil, &depth_stencil_view_desc,&m_depth_stencil_view);

		//
		ZeroMemory(&m_viewport, sizeof(D3D11_VIEWPORT));
		m_viewport.Height   = (float)m_bb_desc.Height;
		m_viewport.Width    = (float)m_bb_desc.Width;
		m_viewport.MinDepth = 0;
		m_viewport.MaxDepth = 1;
		m_d3d_device_context->RSSetViewports(1, &m_viewport);

		return hr;
	}

	HRESULT DeviceResourcesDX::release_backbuffer()
	{
		HRESULT hr = S_OK;

		// Release the render target view based on the back buffer:
		m_render_target->Release();
		m_back_buffer->Release();

		// The depth stencil will need to be resized, so release it (and view):
		m_depth_stencil_view->Release();
		m_depth_stencil->Release();

		// After releasing references to these resources, we need to call Flush() to 
		// ensure that Direct3D also releases any references it might still have to
		// the same resources - such as pipeline bindings.
		m_d3d_device_context->Flush();

		return hr;
	}

	HRESULT DeviceResourcesDX::go_fullscreen(UINT width, UINT height)
	{
		HRESULT hr = S_OK;

		//change state
		hr = m_DXGI_swap_chain->SetFullscreenState(TRUE, NULL);
		
		// Before calling ResizeBuffers, you have to release all references to the back 
		// buffer device resource.
		release_backbuffer();

		// Now we can call ResizeBuffers.
		hr = m_DXGI_swap_chain->ResizeBuffers(
			0,                   // Number of buffers. Set this to 0 to preserve the existing setting.
			width, height,       // Width and height of the swap chain. Set to 0 to match the screen resolution.
			DXGI_FORMAT_UNKNOWN, // This tells DXGI to retain the current back buffer format.
			0
		);

		// Then we can recreate the back buffer, depth buffer, and so on.
		hr = configure_backbuffer();

		//return state
		return hr;
	}

	HRESULT DeviceResourcesDX::go_windowed(UINT width, UINT height)
	{
		HRESULT hr = S_OK;

		//change state
		hr = m_DXGI_swap_chain->SetFullscreenState(FALSE, NULL);

		// Before calling ResizeBuffers, you have to release all references to the back 
		// buffer device resource.
		release_backbuffer();

		// Now we can call ResizeBuffers.
		hr = m_DXGI_swap_chain->ResizeBuffers(
			0,                   // Number of buffers. Set this to 0 to preserve the existing setting.
			width, height,       // Width and height of the swap chain. Set to 0 to match the screen resolution.
			DXGI_FORMAT_UNKNOWN, // This tells DXGI to retain the current back buffer format.
			0
		);

		// Then we can recreate the back buffer, depth buffer, and so on.
		hr = configure_backbuffer();

		//return state
		return hr;
	}
	///////////////////////////////////////////////////////////////////////////////////
	WindowDX::WindowDX(HDC hDC, HWND  hWnd, const WindowInfo& info)
	{
		m_type   = DX_WINDOW;
		m_hDC    = hDC;
		m_hWnd   = hWnd;
		m_info   = info;
		m_device = new DeviceResourcesDX(m_info);

		//set
		enable_resize(info.m_resize);
		//get info
		update_last_window_attributes();
		//build device context
		m_device->create_device_resources(hWnd);
		m_device->configure_backbuffer();
		//set full screen
		enable_fullscreen(info.m_fullscreen);

	}

	void WindowDX::swap() const
	{
		if (m_device)
		{
			m_device->present();
		}
	}

	void WindowDX::acquire_context() const
	{
		//none
	}

	bool WindowDX::is_fullscreen() const
	{
		return m_info.m_fullscreen;
	}

	bool WindowDX::is_resizable() const
	{
		return m_info.m_resize;
	}

	void WindowDX::get_size(unsigned int size[2]) const
	{
		RECT window_box{ 0, 0, 0, 0 };
		GetClientRect(m_hWnd, &window_box);
		//get_rect_of_window_including_aero(m_hWnd, &window_box);
		size[0] = window_box.right - window_box.left;
		size[1] = window_box.bottom - window_box.top;
	}

	void WindowDX::get_position(int position[2]) const
	{
		POINT pos{ 0, 0 };
		ClientToScreen(m_hWnd, &pos);
		position[0] = pos.x;
		position[1] = pos.y;
	}

	void WindowDX::set_size(unsigned int size[2])
	{
		//calc size window
		unsigned int window_real_size[2] = { 0,0 };
		compute_window_size(size, window_real_size);
		//Resize
		if (SetWindowPos(
			m_hWnd
			, HWND_NOTOPMOST
			, m_info.m_position[0]
			, m_info.m_position[1]
			, window_real_size[0]
			, window_real_size[1]
			, SWP_SHOWWINDOW
		))
		{
			//save new size
			m_info.m_size[0] = size[0];
			m_info.m_size[1] = size[1];
		}
	}

	void WindowDX::set_position(int position[2])
	{
		//calc size window
		unsigned int window_real_size[2] = { 0,0 };
		compute_window_size(m_info.m_size, window_real_size);
		//Resize
		if (SetWindowPos(
			m_hWnd
			, HWND_NOTOPMOST
			, position[0]
			, position[1]
			, window_real_size[0]
			, window_real_size[1]
			, SWP_SHOWWINDOW
		))
		{
			//save new position
			m_info.m_position[0] = position[0];
			m_info.m_position[1] = position[1];
		}
	}

	bool WindowDX::enable_resize(bool enable)
	{
		m_info.m_resize = enable;
		LONG style = GetWindowLong(m_hWnd, GWL_STYLE);
		//change style
		if (enable)
		{
			style |= WS_SIZEBOX;
		}
		else
		{
			style &= ~WS_SIZEBOX;
		}
		//set
		return SetWindowLong(m_hWnd, GWL_STYLE, style);
	}

	bool WindowDX::enable_fullscreen(bool enable)
	{
		if (m_info.m_fullscreen == enable) return true;
		//...
		bool is_change_successful;
		//hide window
		ShowWindow(m_hWnd, SW_HIDE);
		//test
		if (enable)
		{
			update_last_window_attributes();

			DEVMODE fullscreen_settings;
			fullscreen_settings.dmSize = sizeof(fullscreen_settings);

			auto* screen = (ScreenWin32*)(m_info.m_screen->conteiner());
			EnumDisplaySettings(screen->m_display_name, 0, &fullscreen_settings);
			fullscreen_settings.dmPelsWidth = m_info.m_size[0];
			fullscreen_settings.dmPelsHeight = m_info.m_size[1];
			fullscreen_settings.dmBitsPerPel = m_info.m_context.m_color;
			fullscreen_settings.dmFields =
				DM_PELSWIDTH
				| DM_PELSHEIGHT
				| DM_BITSPERPEL;

			is_change_successful = ChangeDisplaySettings(&fullscreen_settings, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL;
			//change only if a success
			if (is_change_successful)
			{
				SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TOPMOST);
				SetWindowLongPtr(m_hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
				SetWindowPos(
					m_hWnd
					, HWND_TOPMOST
					, 0
					, 0
					, m_info.m_size[0]
					, m_info.m_size[1]
					, SWP_SHOWWINDOW
				);
				ShowWindow(m_hWnd, SW_MAXIMIZE);
				//update
				m_info.m_fullscreen = enable;
				//device fullscreen mode
				if (m_device) m_device->go_fullscreen(m_info.m_size[0], m_info.m_size[1]);
			}
		}
		else
		{
			is_change_successful = ChangeDisplaySettings(NULL, CDS_RESET) == DISP_CHANGE_SUCCESSFUL;
			//change only if a success
			if (is_change_successful)
			{
				SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, m_last_window_exstyle);
				SetWindowLongPtr(m_hWnd, GWL_STYLE, m_last_window_style);
				//calc size window
				unsigned int last_window_real_size[2] = { 0,0 };
				compute_window_size(m_last_window_size, last_window_real_size);
				//Resize
				SetWindowPos(
					m_hWnd
					, HWND_NOTOPMOST
					, m_last_window_position[0]
					, m_last_window_position[1]
					, last_window_real_size[0]
					, last_window_real_size[1]
					, SWP_SHOWWINDOW
				);
				//update
				m_info.m_fullscreen = enable;
				//device fullscreen mode
				if (m_device) m_device->go_windowed(last_window_real_size[0], last_window_real_size[1]);
			}
		}
		//show window
		ShowWindow(m_hWnd, SW_SHOW);
		//end
		return is_change_successful;
	}

	HWND WindowDX::narive() const
	{
		return m_hWnd;
	}

	DeviceResources* WindowDX::device() const
	{
		return m_device;
	}

	void WindowDX::update_last_window_attributes()
	{
		get_position(m_last_window_position);
		get_size(m_last_window_size);
		m_last_window_exstyle = GetWindowLong(m_hWnd, GWL_EXSTYLE);
		m_last_window_style = GetWindowLong(m_hWnd, GWL_STYLE);
	}

	WindowDX::~WindowDX()
	{
		//release device
		if (m_device)		 delete m_device;
		//release DC
		if (m_hWnd && m_hDC) ReleaseDC(m_hWnd, m_hDC);
		//delete Window
		if (m_hWnd)			 DestroyWindow(m_hWnd);
		//all to null 
		m_device = nullptr;
		m_hDC = nullptr;
		m_hWnd = nullptr;
	}
}
}
}