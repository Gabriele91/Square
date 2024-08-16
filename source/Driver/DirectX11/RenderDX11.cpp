//
//  Square
//
//  Created by Gabriele on 15/08/16.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#include "RenderDX11.h"
#include "Square/Driver/Window.h"
//--------------------------------------------------
#include <iostream>
#include <unordered_map>
//RIGHT HEAND
#define DIRECTX_MODE
//--------------------------------------------------

namespace Square
{
namespace Render
{
	////////////////////////////////////////////////////////////////////////////////////////////////////
	const char* Shader::hlsl_default_header=
	R"HLSL(
	)HLSL";

	const char* Shader::hlsl_header_by_type[ST_N_SHADER] =
	{
		//VERTEX
		"#define saturate(x) clamp( x, 0.0, 1.0 )       \n",
		//FRAGMENT
		"#define saturate(x) clamp( x, 0.0, 1.0 )       \n",
		//GEOMETRY
		"#define saturate(x) clamp( x, 0.0, 1.0 )       \n",
		//TASSELLATION_CONTROL
		"#define saturate(x) clamp( x, 0.0, 1.0 )       \n",
		//TASSELLATION_EVALUATION
		"#define saturate(x) clamp( x, 0.0, 1.0 )       \n",
		//COMPUTE
		"#define saturate(x) clamp( x, 0.0, 1.0 )       \n"
	};

	const char* Shader::hlsl_shader_names[ST_N_SHADER] =
	{
		"VERTEX_SHADER",
		"FRAGMENT_SHADER",
		"GEOMETRY_SHADER",
		"TESS_CONTROL_SHADER",
		"TESS_EVALUATION_SHADER",
		"COMPUTE_SHADER"
	};
    ////////////////////////////////////////////////////////////////////////////////////////////////////
	//uniform
	void UniformDX11::set(Texture* in_texture)
	{
		//bind texture
		m_context->bind_texture(in_texture, (int)m_offset, (int)m_offset);
	}

	void UniformDX11::set(int i)
	{
		m_shader->global_buffer_bind();
		primitive_write(i);
	}
    void UniformDX11::set(float f)
    {
		m_shader->global_buffer_bind();
		primitive_write(f);
    }
    void UniformDX11::set(double d)
    {
		m_shader->global_buffer_bind();
		primitive_write(d);
    }
    
    
    void UniformDX11::set(const IVec2& v2)
    {
		m_shader->global_buffer_bind();
		array_write<int>(value_ptr(v2), 2);
    }
    void UniformDX11::set(const IVec3& v3)
    {
		array_write<int>(value_ptr(v3), 3);
    }
    void UniformDX11::set(const IVec4& v4)
    {
		m_shader->global_buffer_bind();
		array_write<int>(value_ptr(v4), 4);
    }
    
	void UniformDX11::set(const Vec2& v2)
	{
		m_shader->global_buffer_bind();
		array_write(value_ptr(v2), 2);
	}
	void UniformDX11::set(const Vec3& v3)
	{
		m_shader->global_buffer_bind();
		array_write(value_ptr(v3), 3);
	}
	void UniformDX11::set(const Vec4& v4)
	{
		m_shader->global_buffer_bind();
		array_write<float>(value_ptr(v4), 4);
	}
	void UniformDX11::set(const Mat3& m3)
	{
		m_shader->global_buffer_bind();
		array_write<float>(value_ptr(m3), 3 * 3);
	}
	void UniformDX11::set(const Mat4& m4)
	{
		m_shader->global_buffer_bind();
		array_write<float>(value_ptr(m4), 4 * 4);
	}
    
    void UniformDX11::set(const DVec2& v2)
    {
		m_shader->global_buffer_bind();
		array_write<double>(value_ptr(v2), 2);
    }
    void UniformDX11::set(const DVec3& v3)
    {
		m_shader->global_buffer_bind();
		array_write<double>(value_ptr(v3), 3);
    }
    void UniformDX11::set(const DVec4& v4)
    {
		m_shader->global_buffer_bind();
		array_write<double>(value_ptr(v4), 4);
    }
    void UniformDX11::set(const DMat3& m3)
    {
		m_shader->global_buffer_bind();
		array_write<double>(value_ptr(m3), 3 * 3);
    }
    void UniformDX11::set(const DMat4& m4)
    {
		m_shader->global_buffer_bind();
		array_write<double>(value_ptr(m4), 4 * 4);
    }

    void UniformDX11::set(Texture* tvector, size_t n)
    {
        /* 3D texture? */
        //for all
		for (size_t i = 0; i!= n; ++i)
        {
            //bind texture
            m_context->bind_texture(tvector++, (int)m_offset+i, (int)m_offset+i);
        }
    }

	void UniformDX11::set(const int* i, size_t n)
	{
		m_shader->global_buffer_bind();
		array_write(i, n);
	}
    void UniformDX11::set(const float* f, size_t n)
    {
		m_shader->global_buffer_bind();
		array_write(f, n);
    }
    void UniformDX11::set(const double* d, size_t n)
    {
		m_shader->global_buffer_bind();
		array_write(d, n);
    }
    
    void UniformDX11::set(const IVec2* v2, size_t n)
    {
		m_shader->global_buffer_bind();
		array_write(v2, n);
    }
    void UniformDX11::set(const IVec3* v3, size_t n)
    {
		m_shader->global_buffer_bind();
		array_write(v3, n);
    }
    void UniformDX11::set(const IVec4* v4, size_t n)
    {
		m_shader->global_buffer_bind();
		array_write(v4, n);
    }
    
	void UniformDX11::set(const Vec2* v2, size_t n)
	{
		m_shader->global_buffer_bind();
		array_write(v2, n);
	}
	void UniformDX11::set(const Vec3* v3, size_t n)
	{
		m_shader->global_buffer_bind();
		array_write(v3, n);
	}
	void UniformDX11::set(const Vec4* v4, size_t n)
	{
		m_shader->global_buffer_bind();
		array_write(v4, n);
	}
	void UniformDX11::set(const Mat3* m3, size_t n)
	{
		m_shader->global_buffer_bind();
		array_write(m3, n);
	}
	void UniformDX11::set(const Mat4* m4, size_t n)
	{
		m_shader->global_buffer_bind();
		array_write(m4, n);
	}
    
    void UniformDX11::set(const DVec2* v2, size_t n)
    {
		m_shader->global_buffer_bind();
		array_write(v2, n);
    }
    void UniformDX11::set(const DVec3* v3, size_t n)
    {
		m_shader->global_buffer_bind();
		array_write(v3, n);
    }
    void UniformDX11::set(const DVec4* v4, size_t n)
    {
		m_shader->global_buffer_bind();
		array_write(v4, n);
    }
    void UniformDX11::set(const DMat3* m3, size_t n)
    {
		m_shader->global_buffer_bind();
		array_write(m3, n);
    }
    void UniformDX11::set(const DMat4* m4, size_t n)
    {
		m_shader->global_buffer_bind();
		array_write(m4, n);
    }

    void UniformDX11::set(const std::vector < Texture* >& t)
    {
        set((Texture*)t.data(), t.size());
    }
	void UniformDX11::set(const std::vector < int >& i)
	{
		set(i.data(), i.size());
	}
    void UniformDX11::set(const std::vector < float >& f)
    {
        set(f.data(), f.size());
    }
    void UniformDX11::set(const std::vector < double >& d)
    {
        set(d.data(), d.size());
    }
    
    void UniformDX11::set(const std::vector < IVec2 >& v2)
    {
        set(v2.data(), v2.size());
    }
    void UniformDX11::set(const std::vector < IVec3 >& v3)
    {
        set(v3.data(), v3.size());
    }
    void UniformDX11::set(const std::vector < IVec4 >& v4)
    {
        set(v4.data(), v4.size());
    }
    
	void UniformDX11::set(const std::vector < Vec2 >& v2)
	{
		set(v2.data(), v2.size());
	}
	void UniformDX11::set(const std::vector < Vec3 >& v3)
	{
		set(v3.data(), v3.size());
	}
	void UniformDX11::set(const std::vector < Vec4 >& v4)
	{
		set(v4.data(), v4.size());
	}
	void UniformDX11::set(const std::vector < Mat3 >& m3)
	{
		set(m3.data(), m3.size());
	}
	void UniformDX11::set(const std::vector < Mat4 >& m4)
	{
		set(m4.data(), m4.size());
	}
    
    void UniformDX11::set(const std::vector < DVec2 >& v2)
    {
        set(v2.data(), v2.size());
    }
    void UniformDX11::set(const std::vector < DVec3 >& v3)
    {
        set(v3.data(), v3.size());
    }
    void UniformDX11::set(const std::vector < DVec4 >& v4)
    {
        set(v4.data(), v4.size());
    }
    void UniformDX11::set(const std::vector < DMat3 >& m3)
    {
        set(m3.data(), m3.size());
    }
    void UniformDX11::set(const std::vector < DMat4 >& m4)
    {
        set(m4.data(), m4.size());
    }
	bool UniformDX11::is_valid() { return m_context && m_shader && m_buffer; }
	
	Shader* UniformDX11::get_shader()
	{
		return m_shader;
	}

	UniformDX11::UniformDX11(ContextDX11* context, Shader* shader, unsigned char* buffer, size_t offset)
	: m_context(context)
	, m_shader(shader)
	, m_buffer(buffer)
	, m_offset(offset)
	{
	}

	UniformDX11::UniformDX11()
	{
	}
	
	UniformDX11::~UniformDX11()
	{
		//none
	}
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    UniformConstBufferDX11::UniformConstBufferDX11(ContextDX11* context, Shader* shader, const std::string& cname)
    : m_context(context)
	, m_shader(shader)
    {
		for (int s_type = 0; s_type != ST_N_SHADER; ++s_type)
		{
			//init
			m_slots[s_type] = -1;
			//shader?
			if (!shader->binary((ShaderType)s_type)) continue;
			//reflector
			ID3D11ShaderReflection *reflector = nullptr;
			D3DReflect(shader->binary((ShaderType)s_type)->GetBufferPointer()
					 , shader->binary((ShaderType)s_type)->GetBufferSize()
				     , IID_ID3D11ShaderReflection
				     , (void **)&reflector);
			D3D11_SHADER_DESC desc_shader;
			reflector->GetDesc(&desc_shader);
			//search
			for (UINT ic = 0; ic < desc_shader.ConstantBuffers; ++ic)
			{
				//shader desc
				D3D11_SHADER_BUFFER_DESC desc;
				reflector->GetConstantBufferByIndex(ic)->GetDesc(&desc);
				//compare
				if (std::strcmp(desc.Name, cname.data()) == 0)
				{   
					//bind
					D3D11_SHADER_INPUT_BIND_DESC bind_desc;
					reflector->GetResourceBindingDescByName(desc.Name, &bind_desc);
					//slot
					m_slots[s_type] = bind_desc.BindPoint;
					m_found_the_cbuffer = true;
				}
			}
		}
    }
	UniformConstBufferDX11::UniformConstBufferDX11()
    {
		for (int s_type = 0; s_type != ST_N_SHADER; ++s_type) m_slots[s_type] = -1;
    }
	UniformConstBufferDX11::~UniformConstBufferDX11()
    {
        //none
    }
    //bind
	void UniformConstBufferDX11::bind(const ConstBuffer* buffer)
	{
		#define CBBIND(T, type)  \
			if (m_slots[type] >= 0) m_context->device_context()-> T ## SetConstantBuffers(m_slots[type], 1, &buffer->m_buffer);
		CBBIND(VS, ST_VERTEX_SHADER)
		CBBIND(PS, ST_FRAGMENT_SHADER)
		CBBIND(GS, ST_GEOMETRY_SHADER)
		CBBIND(HS, ST_TASSELLATION_CONTROL_SHADER)
		CBBIND(DS, ST_TASSELLATION_EVALUATION_SHADER)
		CBBIND(CS, ST_COMPUTE_SHADER)
		#undef CBBIND
	}
    void UniformConstBufferDX11::unbind()
    {
		#define UNBIND(T, type)  \
			if (m_slots[type] >= 0) m_context->device_context()-> T ## SetConstantBuffers(m_slots[type], 0, nullptr);
		UNBIND(VS, ST_VERTEX_SHADER)
		UNBIND(PS, ST_FRAGMENT_SHADER)
		UNBIND(GS, ST_GEOMETRY_SHADER)
		UNBIND(HS, ST_TASSELLATION_CONTROL_SHADER)
		UNBIND(DS, ST_TASSELLATION_EVALUATION_SHADER)
		UNBIND(CS, ST_COMPUTE_SHADER)
		#undef UNBIND
    }
    //buffer info
    bool UniformConstBufferDX11::is_valid()
    {
        return m_shader && m_found_the_cbuffer;
    }
    Shader* UniformConstBufferDX11::get_shader()
    {
        return m_shader;
    }
    
    ////////////////////
	//     RENDER     //
	////////////////////
	ID3D11Device* ContextDX11::device() const { return m_device; }
	IDXGISwapChain* ContextDX11::swap() const { return m_swap; }
	ID3D11DeviceContext* ContextDX11::device_context() const { return m_device_context; }
    ////////////////////
	static const char* dx_vendor(ContextDX11* context)
	{
		if (auto* device = context->device())
		{
			IDXGIDevice*      dx_gi_device = nullptr;
			IDXGIAdapter*     adapter = nullptr;
			DXGI_ADAPTER_DESC adapter_desc;
			//given device
			if (SUCCEEDED(device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dx_gi_device)))
			{
				if (SUCCEEDED(dx_gi_device->GetAdapter(&adapter)))
				{
					if (SUCCEEDED(dx_gi_device->GetAdapter(&adapter)))
					{
						adapter->GetDesc(&adapter_desc);
						if (wcsstr(adapter_desc.Description, L"Intel"))
							return "Intel";
						else if (wcsstr(adapter_desc.Description, L"NVIDIA"))
							return "NVIDIA";
						else if (wcsstr(adapter_desc.Description, L"AMD"))
							return "AMD";
					}
					adapter->Release();
				}
				dx_gi_device->Release();
			}
		}
		return "";
	}

	static inline size_t multiple_of_16(size_t size) 
	{
		return ((size / 16) + ((size % 16) != 0)) * 16;
	}

    static void compute_render_driver_info(ContextDX11* context)
    {
        RenderDriverInfo m_info;
        //type
		context->s_render_driver_info.m_render_driver = DR_DIRECTX;
        //Get driver vendor
		context->s_render_driver_info.m_name = (const char*)dx_vendor(context);
        //shader type
		context->s_render_driver_info.m_shader_language = "HLSL";
        //shader version
		context->s_render_driver_info.m_shader_version  = 50;
		//DX11 version
		context->s_render_driver_info.m_major_version = 11;
		context->s_render_driver_info.m_minor_version = 0;
	}
	
	bool ContextDX11::get_devices(Video::DeviceResources* resource)
	{
		m_device = (ID3D11Device*)resource->get_device();
		m_device_context = (ID3D11DeviceContext*)resource->get_device_context();
		m_swap = (IDXGISwapChain*)resource->get_device_context();
		return true;
	}

	bool ContextDX11::get_view_target(Video::DeviceResources* resource)
	{
		//delete last view target
		if (m_view_target) delete m_view_target;
		//target
		m_view_target = new Target(false);

		m_view_target->m_views.push_back((ID3D11RenderTargetView*)resource->get_render_target());
		m_view_target->m_depth = (ID3D11DepthStencilView*)resource->get_depth_stencil_target();

		m_view_target->m_view_textures.push_back((ID3D11Texture2D*)resource->get_render_resource());
		m_view_target->m_depth_texture = (ID3D11Texture2D*)resource->get_depth_stencil_resource();

		return true;
	}

	bool ContextDX11::build_query_buffer()
	{
		D3D11_TEXTURE2D_DESC scd
		{
			1,//UINT Width;
			1,//UINT Height;
			1,//UINT MipLevels;
			1,//UINT ArraySize;
			DXGI_FORMAT_R8G8B8A8_UNORM,	   //DXGI_FORMAT Format;DXGI_FORMAT_R8G8B8A8_UNORM
			1, 0,                          //DXGI_SAMPLE_DESC SampleDesc;
			D3D11_USAGE_STAGING,           //D3D10_USAGE Usage;
			0,					           //UINT BindFlags;
			D3D11_CPU_ACCESS_READ,         //UINT CPUAccessFlags;
			0					           //UINT MiscFlags;
		};
		m_query_buffer[0] = new Texture2D();
		if (!SUCCEEDED(device()->CreateTexture2D(&scd, NULL, &m_query_buffer[0]->m_texture2D))) return false;
		//default format
		DXGI_FORMAT depth_format = DXGI_FORMAT_D32_FLOAT;
		//get format depth buffer
		if (m_view_target && m_view_target->m_depth_texture)
		{
			D3D11_TEXTURE2D_DESC sdd_screen_buffer;
			m_view_target->m_depth_texture->GetDesc(&sdd_screen_buffer);
			depth_format = sdd_screen_buffer.Format;
		}
		//Depth buffer
		D3D11_TEXTURE2D_DESC sdd
		{
			1,//UINT Width;
			1,//UINT Height;
			1,//UINT MipLevels;
			1,//UINT ArraySize;
			depth_format,	   	    //DXGI_FORMAT Format;
			1, 0,                   //DXGI_SAMPLE_DESC SampleDesc;
			D3D11_USAGE_STAGING,    //D3D10_USAGE Usage;
			0,					    //UINT BindFlags;
			D3D11_CPU_ACCESS_READ,  //UINT CPUAccessFlags;
			0					    //UINT MiscFlags;
		};
		m_query_buffer[1] = new Texture2D();
		if (!SUCCEEDED(device()->CreateTexture2D(&sdd, NULL, &m_query_buffer[1]->m_texture2D))) return false;
		return true;
	}

	bool ContextDX11::dx_op_success(HRESULT h) const
	{
		if (FAILED(h))
		{
			const char* dxerror = "";
			switch (h)
			{
		    #define STRERR_CASE(x) case x: dxerror = #x; break;
			STRERR_CASE(D3D11_ERROR_FILE_NOT_FOUND)
			STRERR_CASE(D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS)
			STRERR_CASE(D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS)
			STRERR_CASE(D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD)
			STRERR_CASE(DXGI_ERROR_INVALID_CALL)
			STRERR_CASE(DXGI_ERROR_WAS_STILL_DRAWING)
			STRERR_CASE(E_FAIL)
			STRERR_CASE(E_INVALIDARG)
			STRERR_CASE(E_OUTOFMEMORY)
			STRERR_CASE(E_NOTIMPL)
			STRERR_CASE(S_FALSE)
			STRERR_CASE(S_OK)
			default: 	break;
			}
			static char s_str[255] = {};
			sprintf_s(s_str, "directx error with HRESULT of %08X (%s)", static_cast<unsigned int>(h), dxerror);
			m_errors.push_back({ s_str });
			return false;
		}
		return true;
	}


	bool ContextDX11::init(Video::DeviceResources* resource)
	{
		//disable vsync
		resource->set_vsync(false);
		//save ref to device resouces
		m_device_resouces = resource;
		//window
		if (!get_devices(resource)) { close(); return false; }
        //get info
		compute_render_driver_info(this);
		//view target update
		auto set_view_target = [this](Video::DeviceResources* resource)
		{
			//target
			if (!get_view_target(resource)) { close(); return false; }
			//set view target
			enable_render_target(m_view_target);
			// OK
			return true;
		};
		//target
		set_view_target(resource);
		//hander callback
		resource->callback_target_changed(set_view_target);
		//build cullface states
		if (!build_cullface_states()) { m_errors.push_back({ "can't build cullfaces" }); close(); return false; }
		//query
		if (!build_query_buffer()) { m_errors.push_back({ "can't build query buffers" });  close(); return false; }
		///////////////////////////////////////////////////////////////////////////////
		// set default state
		// save cullface state
		s_render_state.m_cullface.m_cullface = CF_BACK;
		// save depth state
		s_render_state.m_depth.m_mode = DM_ENABLE_AND_WRITE;
		s_render_state.m_depth.m_type = DT_LESS;
		// save blend state
		s_render_state.m_blend.m_enable = false;
		s_render_state.m_blend.m_src = BLEND_ONE;
		s_render_state.m_blend.m_dst = BLEND_ZERO;
		// save viewport state
		s_render_state.m_viewport.m_viewport = Vec4(0, 0, 0, 0);
		// save clear color state
		s_render_state.m_clear_color.m_color = Vec4(0.1, 0.5, 1.0, 1.0);
		///////////////////////////////////////////////////////////////////////////////
		device_context()->RSSetState(cullface_state(s_render_state.m_cullface));
		device_context()->OMSetDepthStencilState(depth_state(s_render_state.m_depth), 1);
		device_context()->OMSetBlendState(blend_state(s_render_state.m_blend), 0, 0xFFFFFFFFFF);
		D3D11_VIEWPORT vp;
		vp.Width    = s_render_state.m_viewport.m_viewport.z;
		vp.Height   = s_render_state.m_viewport.m_viewport.w;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = s_render_state.m_viewport.m_viewport.x;
		vp.TopLeftY = s_render_state.m_viewport.m_viewport.y;
		device_context()->RSSetViewports(1, &vp);
		///////////////////////////////////////////////////////////////////////////////
		//clear all errors
		print_errors();
		//return
		return true;
	}

	void ContextDX11::print_info()
	{
		std::cout << "Renderer: " << s_render_driver_info.m_name << std::endl;
		std::cout << "DirectX version supported: " 
			<< s_render_driver_info.m_major_version
			<< '.'  
			<< s_render_driver_info.m_minor_version 
			<< std::endl;
	}

	void ContextDX11::close()
	{
		for (auto blend_pair : m_blend_states) blend_pair.second->Release();
		for (auto depth_pair : m_depth_states) depth_pair.second->Release();

		if (m_render_state_cullface_back) m_render_state_cullface_back->Release();
		if (m_render_state_cullface_front) m_render_state_cullface_front->Release();
		if (m_render_state_cullface_back_and_front) m_render_state_cullface_back_and_front->Release();
		if (m_render_state_cullface_disable) m_render_state_cullface_disable->Release();
		
		if (m_view_target) delete m_view_target;
		if (m_query_buffer[0]) delete m_query_buffer[0];
		if (m_query_buffer[1]) delete m_query_buffer[1];

		/* Resource managed by window device
		if (m_device_context) m_device_context->Release();
		if (m_swap)           m_swap->Release();
		if (m_device)         m_device->Release();
		*/

		//to nullptr
		m_blend_states.clear();
		m_depth_states.clear();

		m_render_state_cullface_back = nullptr;
		m_render_state_cullface_front = nullptr;
		m_render_state_cullface_back_and_front = nullptr;
		m_render_state_cullface_disable = nullptr;
		
		m_view_target = nullptr;
		m_query_buffer[0] = nullptr;
		m_query_buffer[1] = nullptr;

		m_device_context = nullptr;
		m_swap = nullptr;
		m_device = nullptr;

		if (m_device_resouces)
		{
			m_device_resouces->callback_target_changed(nullptr);
			m_device_resouces = nullptr;
		}
	}

	RenderDriver ContextDX11::get_render_driver()
	{
		return s_render_driver_info.m_render_driver;
	}
        
    RenderDriverInfo ContextDX11::get_render_driver_info()
    {
        return s_render_driver_info;
    }

	const ClearColorState& ContextDX11::get_clear_color_state()
	{
		return s_render_state.m_clear_color;
	}
        
	void ContextDX11::set_clear_color_state(const ClearColorState& clear_color)
	{
		if (s_render_state.m_clear_color != clear_color)
		{
			s_render_state.m_clear_color = clear_color;
		}
	}

	void ContextDX11::clear(int type)
	{
		if (type & CLEAR_COLOR)
		{
			for (auto view : s_bind_context.m_render_target->m_views)
			{
				device_context()->ClearRenderTargetView(view, value_ptr(s_render_state.m_clear_color.m_color));
			}
		}
        if(type & CLEAR_DEPTH)
			device_context()->ClearDepthStencilView(s_bind_context.m_render_target->m_depth, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	static D3D11_COMPARISON_FUNC get_depth_comp_func(DepthFuncType type)
	{
		switch (type)
		{
		default:
		case DT_LESS:         return D3D11_COMPARISON_LESS;
		case DT_NEVER:        return D3D11_COMPARISON_NEVER;
		case DT_EQUAL:        return D3D11_COMPARISON_EQUAL;
		case DT_GREATER:      return D3D11_COMPARISON_GREATER;
		case DT_NOT_EQUAL:    return D3D11_COMPARISON_NOT_EQUAL;
		case DT_LESS_EQUAL:   return D3D11_COMPARISON_LESS_EQUAL;
		case DT_GREATER_EQUAL:return D3D11_COMPARISON_GREATER_EQUAL;
		case DT_ALWAYS:       return D3D11_COMPARISON_ALWAYS;
		}
	}	
	static D3D11_DEPTH_WRITE_MASK get_depth_write_mask(DepthMode type)
	{
		switch (type)
		{
		default:
		case DM_ENABLE_AND_WRITE:   return D3D11_DEPTH_WRITE_MASK_ALL;
		case DM_DISABLE:
		case DM_ENABLE_ONLY_READ:   return D3D11_DEPTH_WRITE_MASK_ZERO;
		}
	}
	
	ID3D11DepthStencilState* ContextDX11::depth_state(DepthBufferState dbs)
	{
		//search
		auto it_dbs = m_depth_states.find(dbs);
		if (it_dbs != m_depth_states.end()) return it_dbs->second;
		//ENABLE/DISABLE
		bool enable = dbs.m_mode != DM_DISABLE;
		D3D11_STENCIL_OP front_face = enable ? D3D11_STENCIL_OP_INCR : D3D11_STENCIL_OP_KEEP;
		D3D11_STENCIL_OP back_face  = enable ? D3D11_STENCIL_OP_DECR : D3D11_STENCIL_OP_KEEP;
		D3D11_COMPARISON_FUNC back_front_face_comp = enable ? D3D11_COMPARISON_ALWAYS : D3D11_COMPARISON_NEVER;
		//else build
		D3D11_DEPTH_STENCIL_DESC dsd
		{

			    enable,                          // BOOL DepthEnable;
				get_depth_write_mask(dbs.m_mode),// D3D11_DEPTH_WRITE_MASK DepthWriteMask;
				get_depth_comp_func(dbs.m_type), // D3D11_COMPARISON_FUNC DepthFunc;
				enable,                          // BOOL StencilEnable;
				(UINT8)0xFFFFFFFF * enable,      // UINT8 StencilReadMask;
				(UINT8)0xFFFFFFFF * enable,      // UINT8 StencilWriteMask;
				{                                // D3D11_DEPTH_STENCILOP_DESC FrontFace;
					D3D11_STENCIL_OP_KEEP,       // D3D11_STENCIL_OP StencilFailOp;
					front_face,					 // D3D11_STENCIL_OP StencilDepthFailOp;
					D3D11_STENCIL_OP_KEEP,       // D3D11_STENCIL_OP StencilPassOp;
					back_front_face_comp,		 // D3D11_COMPARISON_FUNC StencilFunc;
				},
				{                                // D3D11_DEPTH_STENCILOP_DESC BackFace;
					D3D11_STENCIL_OP_KEEP,       // D3D11_STENCIL_OP StencilFailOp;
					back_face,					 // D3D11_STENCIL_OP StencilDepthFailOp;
					D3D11_STENCIL_OP_KEEP,       // D3D11_STENCIL_OP StencilPassOp;
					back_front_face_comp,		 // D3D11_COMPARISON_FUNC StencilFunc;
				},
		};
		ID3D11DepthStencilState* depth_stencil_state;
		if (dx_op_success(device()->CreateDepthStencilState(&dsd, &depth_stencil_state)))
		{
			m_depth_states[dbs] = depth_stencil_state;
			return depth_stencil_state;
		}
		return nullptr;
	}

	const DepthBufferState& ContextDX11::get_depth_buffer_state()
	{
		return s_render_state.m_depth;
	}

	void ContextDX11::set_depth_buffer_state(const DepthBufferState& depth)
	{

		if (s_render_state.m_depth != depth)
		{
			s_render_state.m_depth = depth;
			device_context()->OMSetDepthStencilState(depth_state(depth), 1);
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool ContextDX11::build_cullface_states()
	{
		D3D11_RASTERIZER_DESC rds;
		ZeroMemory(&rds, sizeof(D3D11_RASTERIZER_DESC));
		rds.FillMode = D3D11_FILL_SOLID;
		rds.CullMode = D3D11_CULL_BACK;
		rds.DepthClipEnable = true;
		rds.FrontCounterClockwise = true;
		rds.DepthBias = false;
		rds.DepthBiasClamp = 0;
		rds.SlopeScaledDepthBias = 0;
		if (!SUCCEEDED(device()->CreateRasterizerState(&rds, &m_render_state_cullface_back))) return false;
		rds.CullMode = D3D11_CULL_FRONT;
		if (!SUCCEEDED(device()->CreateRasterizerState(&rds, &m_render_state_cullface_front))) return false;
		rds.CullMode = D3D11_CULL_NONE;
		if (!SUCCEEDED(device()->CreateRasterizerState(&rds, &m_render_state_cullface_disable))) return false;
		rds.FillMode = D3D11_FILL_WIREFRAME;
		if (!SUCCEEDED(device()->CreateRasterizerState(&rds, &m_render_state_cullface_back_and_front))) return false;
		return true;
	}

	ID3D11RasterizerState* ContextDX11::cullface_state(CullfaceState cullface)
	{
		switch (cullface.m_cullface)
		{
		case CF_BACK:           return m_render_state_cullface_back;            break;
		case CF_FRONT:          return m_render_state_cullface_front;           break;
		case CF_FRONT_AND_BACK: return m_render_state_cullface_back_and_front;  break;
		case CF_DISABLE:        return m_render_state_cullface_disable;         break;
		default: return nullptr;  break;
		}
	}

	const CullfaceState& ContextDX11::get_cullface_state()
	{
		return s_render_state.m_cullface;
	}

	void ContextDX11::set_cullface_state(const CullfaceState& cfs)
	{
		if (s_render_state.m_cullface != cfs)
		{
			s_render_state.m_cullface = cfs;
			device_context()->RSSetState(cullface_state(cfs));
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	const ViewportState& ContextDX11::get_viewport_state()
	{
		//return
		return s_render_state.m_viewport;
	}

	void  ContextDX11::set_viewport_state(const ViewportState& vs)
	{
		if (s_render_state.m_viewport != vs)
		{
			s_render_state.m_viewport = vs;
			D3D11_VIEWPORT vp;
			vp.Width = vs.m_viewport.z;
			vp.Height = vs.m_viewport.w;
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = vs.m_viewport.x;
			vp.TopLeftY = vs.m_viewport.y;
			device_context()->RSSetViewports(1, &vp);
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	static D3D11_BLEND get_blend_type(BlendType type, bool isAlpha) 
	{
		switch (type)
		{
		default:
		case BLEND_ZERO: return D3D11_BLEND_ZERO;
		case BLEND_ONE:  return D3D11_BLEND_ONE;

		case BLEND_ONE_MINUS_DST_COLOR: return D3D11_BLEND_INV_DEST_COLOR;
		case BLEND_ONE_MINUS_DST_ALPHA: return D3D11_BLEND_INV_DEST_ALPHA;
		case BLEND_ONE_MINUS_SRC_COLOR: return D3D11_BLEND_INV_SRC_COLOR;
		case BLEND_ONE_MINUS_SRC_ALPHA: return D3D11_BLEND_INV_SRC_ALPHA;

		case BLEND_DST_COLOR: return D3D11_BLEND_DEST_COLOR;
		case BLEND_DST_ALPHA: return D3D11_BLEND_DEST_ALPHA;

		case BLEND_SRC_COLOR: return D3D11_BLEND_SRC_COLOR;
		case BLEND_SRC_ALPHA: return D3D11_BLEND_SRC_ALPHA;
		case BLEND_SRC_ALPHA_SATURATE: return D3D11_BLEND_SRC_ALPHA_SAT;
		}
	}

	ID3D11BlendState* ContextDX11::blend_state(BlendState dbs)
	{
		//search
		auto it_dbs = m_blend_states.find(dbs);
		if (it_dbs != m_blend_states.end()) return it_dbs->second;
		//ENABLE/DISABLE
		D3D11_BLEND_DESC bsd;
		ZeroMemory(&bsd, sizeof(D3D11_BLEND_DESC));
		//ALPHA
		bsd.AlphaToCoverageEnable = TRUE;
		//FOR ALL
		bsd.IndependentBlendEnable = FALSE; //USE only RenderTarget[0] 
		//for (D3D11_RENDER_TARGET_BLEND_DESC& btarget : bsd.RenderTarget)
		D3D11_RENDER_TARGET_BLEND_DESC& btarget = bsd.RenderTarget[0];
		{
			btarget.BlendEnable = dbs.m_enable;
			btarget.SrcBlend = get_blend_type(dbs.m_src, 0);
			btarget.DestBlend = get_blend_type(dbs.m_dst, 0);
			btarget.BlendOp = D3D11_BLEND_OP_ADD;
			btarget.SrcBlendAlpha = get_blend_type(dbs.m_src, 1);
			btarget.DestBlendAlpha = get_blend_type(dbs.m_src, 1);
			btarget.BlendOpAlpha = D3D11_BLEND_OP_ADD;
			btarget.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		}
		ID3D11BlendState* blend_state;
		if (dx_op_success(device()->CreateBlendState(&bsd, &blend_state)))
		{
			m_blend_states[dbs] = blend_state;
			return blend_state;
		}
		return nullptr;
	}

	const State& ContextDX11::get_render_state()
	{
		return s_render_state;
	}

	void  ContextDX11::set_blend_state(const BlendState& bs)
	{
		if (s_render_state.m_blend != bs)
		{
			//save blend
			s_render_state.m_blend = bs;
			//set state
			device_context()->OMSetBlendState(blend_state(bs), 0, 0xFFFFFFFFFF);
		}
	}

	const BlendState& ContextDX11::get_blend_state()
	{
		return s_render_state.m_blend;
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void ContextDX11::set_render_state(const State& rs)
	{
		set_clear_color_state(rs.m_clear_color);
		set_cullface_state(rs.m_cullface);
		set_viewport_state(rs.m_viewport);
		set_depth_buffer_state(rs.m_depth);
		set_blend_state(rs.m_blend);
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	//projection matrix
	/*
	VBO & IBO
	*/
	ConstBuffer* ContextDX11::create_stream_CB(const unsigned char* data, size_t size)
	{
		//desc
		D3D11_BUFFER_DESC cbd;
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.ByteWidth = multiple_of_16(size);
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.CPUAccessFlags = /* D3D11_CPU_ACCESS_READ | */ D3D11_CPU_ACCESS_WRITE;
		cbd.MiscFlags = 0;
		cbd.StructureByteStride = 0;
		D3D11_SUBRESOURCE_DATA init_data;
		init_data.pSysMem = data;
		init_data.SysMemPitch = 0;
		init_data.SysMemSlicePitch = 0;
		//ref
		const D3D11_SUBRESOURCE_DATA* subresource = data ? &init_data : nullptr;
		//buffer
		ID3D11Buffer* buffer;
		//success
		if (dx_op_success(device()->CreateBuffer(&cbd, subresource, &buffer)))
		{
			return new ConstBuffer(buffer, size);
		}
		return nullptr;
	}

	VertexBuffer* ContextDX11::create_stream_VBO(const unsigned char* vbo, size_t stride, size_t n)
	{
		size_t size = stride * n;
		//desc
		D3D11_BUFFER_DESC vbd;
		vbd.Usage = D3D11_USAGE_DYNAMIC;
		vbd.ByteWidth = multiple_of_16(size);
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = /* D3D11_CPU_ACCESS_READ | */  D3D11_CPU_ACCESS_WRITE;
		vbd.MiscFlags = 0;
		vbd.StructureByteStride = 0;
		D3D11_SUBRESOURCE_DATA init_data;
		init_data.pSysMem = vbo;
		init_data.SysMemPitch = 0;
		init_data.SysMemSlicePitch = 0;
		//buffer
		ID3D11Buffer* buffer;
		//success
		if (dx_op_success(device()->CreateBuffer(&vbd, vbo ? &init_data : nullptr, &buffer)))
		{
			return new VertexBuffer(buffer, stride, n);
		}
		return nullptr;
	}

	IndexBuffer* ContextDX11::create_stream_IBO(const unsigned int* ibo, size_t n)
	{
		size_t size = sizeof(unsigned int) * n;
		//desc
		D3D11_BUFFER_DESC ibd;
		ibd.Usage = D3D11_USAGE_DYNAMIC;
		ibd.ByteWidth = multiple_of_16(size);
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = /* D3D11_CPU_ACCESS_READ | */  D3D11_CPU_ACCESS_WRITE;
		ibd.MiscFlags = 0;
		ibd.StructureByteStride = 0;
		D3D11_SUBRESOURCE_DATA init_data;
		init_data.pSysMem = ibo;
		init_data.SysMemPitch = 0;
		init_data.SysMemSlicePitch = 0;
		//buffer
		ID3D11Buffer* buffer;
		//success
		if (dx_op_success(device()->CreateBuffer(&ibd, ibo ? &init_data : nullptr, &buffer)))
		{
			return new IndexBuffer(buffer, size);
		}
		return nullptr;
	}
		
	ConstBuffer* ContextDX11::create_CB(const unsigned char* data, size_t size)
	{
		//desc
		D3D11_BUFFER_DESC cbd;
		cbd.Usage = D3D11_USAGE_DEFAULT;
		cbd.ByteWidth = multiple_of_16(size);
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.CPUAccessFlags = 0;
		cbd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA init_data;
		init_data.pSysMem = data;
		init_data.SysMemPitch = 0;
		init_data.SysMemSlicePitch = 0;
		//ref
		const D3D11_SUBRESOURCE_DATA* subresource = data ? &init_data : nullptr;
		//buffer
		ID3D11Buffer* buffer;
		//success
		if (dx_op_success(device()->CreateBuffer(&cbd, subresource, &buffer)))
		{
			return new ConstBuffer(buffer, size);
		}
		return nullptr;
	}

	VertexBuffer* ContextDX11::create_VBO(const unsigned char* vbo, size_t stride, size_t n)
	{
		size_t size = stride * n;
		//desc
		D3D11_BUFFER_DESC vbd;
		vbd.Usage = D3D11_USAGE_DEFAULT;
		vbd.ByteWidth = multiple_of_16(size);
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		vbd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA init_data;
		init_data.pSysMem = vbo;
		init_data.SysMemPitch = 0;
		init_data.SysMemSlicePitch = 0;
		//buffer
		ID3D11Buffer* buffer;
		//success
		if (dx_op_success(device()->CreateBuffer(&vbd, vbo ? &init_data : nullptr, &buffer)))
		{
			return new VertexBuffer(buffer, stride, n);
		}		
		return nullptr;
	}

	IndexBuffer* ContextDX11::create_IBO(const unsigned int* ibo, size_t n)
	{
		size_t size = sizeof(unsigned int) * n;
		//desc
		D3D11_BUFFER_DESC ibd;
		ibd.Usage = D3D11_USAGE_DEFAULT;
		ibd.ByteWidth = multiple_of_16(size);
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA init_data;
		init_data.pSysMem = ibo;
		init_data.SysMemPitch = 0;
		init_data.SysMemSlicePitch = 0;
		//buffer
		ID3D11Buffer* buffer;
		//success
		if (dx_op_success(device()->CreateBuffer(&ibd, ibo ? &init_data : nullptr, &buffer)))
		{
			return new IndexBuffer(buffer, size);
		}
		return nullptr;
	}

	uint64 ContextDX11::get_native_CB(const ConstBuffer* cb) const
	{
		return (uint64)cb->m_buffer;
	}

	uint64 ContextDX11::get_native_VBO(const VertexBuffer* vb) const
	{
		return (uint64)vb->m_buffer;
	}

	uint64 ContextDX11::get_native_IBO(const IndexBuffer* ib) const
	{
		return (uint64)ib->m_buffer;
	}
        
    size_t ContextDX11::get_size_CB(const ConstBuffer* cb) const
    {
        return cb->get_size();
    }
    
    size_t ContextDX11::get_size_VBO(const VertexBuffer* vb) const
    {
        return vb->get_size();
    }
    
    size_t ContextDX11::get_size_IBO(const IndexBuffer* ib) const
    {
        return ib->get_size();
    }

	void ContextDX11::update_steam_CB(ConstBuffer* cb, const unsigned char* data, size_t size)
	{
		D3D11_MAPPED_SUBRESOURCE source;
		device_context()->Map(cb->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &source);
		memcpy(source.pData, (void*)data, size);
		device_context()->Unmap(cb->m_buffer, 0);
	}

	void ContextDX11::update_steam_VBO(VertexBuffer* vbo, const unsigned char* data, size_t size)
	{
		D3D11_MAPPED_SUBRESOURCE source;
		device_context()->Map(vbo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &source);
		memcpy(source.pData, (void*)data, size);
		device_context()->Unmap(vbo->m_buffer, 0);
	}

	void ContextDX11::update_steam_IBO(IndexBuffer* ibo, const unsigned int* data, size_t n) 
	{
		D3D11_MAPPED_SUBRESOURCE source;
		device_context()->Map(ibo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &source);
		memcpy(source.pData, (void*)data, sizeof(unsigned int)*n);
		device_context()->Unmap(ibo->m_buffer, 0);
	}
		 
	void ContextDX11::bind_CB(ConstBuffer* cb)
	{
		if (cb && s_bind_context.m_const_buffer != cb)
		{
			//unbind
			if (s_bind_context.m_const_buffer)
			{
				unbind_CB(s_bind_context.m_const_buffer);
			}
			//bind
			/* not use */
			//update
			s_bind_context.m_const_buffer = cb;
		}
	}

	void ContextDX11::bind_VBO(VertexBuffer* vbo)
	{
        if(vbo && s_bind_context.m_vertex_buffer != vbo)
        {
            //unbind
            if(s_bind_context.m_vertex_buffer)
            {
                unbind_VBO(s_bind_context.m_vertex_buffer);
            }
			//set into dx context
			device_context()->IASetVertexBuffers(0, 1, &vbo->m_buffer, &vbo->m_stride, &vbo->m_offset);
            //update
            s_bind_context.m_vertex_buffer = vbo;
        }
	}

	void ContextDX11::bind_IBO(IndexBuffer* ibo)
    {
        if(ibo && s_bind_context.m_index_buffer != ibo)
        {
            //unbind
            if(s_bind_context.m_index_buffer)
            {
                unbind_IBO(s_bind_context.m_index_buffer);
            }
			//set into dx context
			device_context()->IASetIndexBuffer(ibo->m_buffer, DXGI_FORMAT_R32_UINT, 0);
            //update
            s_bind_context.m_index_buffer = ibo;
        }
	}

	void ContextDX11::unbind_CB(ConstBuffer* cb)
	{
		if (cb)
		{
			square_assert(s_bind_context.m_const_buffer == cb);
			/* not use */
			s_bind_context.m_const_buffer = nullptr;
		}
	}

	void ContextDX11::unbind_VBO(VertexBuffer* vbo)
	{
        if(vbo)
        {
            square_assert(s_bind_context.m_vertex_buffer == vbo);
			device_context()->IASetVertexBuffers(0, 0, nullptr, 0, 0);
            s_bind_context.m_vertex_buffer = nullptr;
        }
	}

	void ContextDX11::unbind_IBO(IndexBuffer* ibo)
    {
        if(ibo)
        {
            square_assert(s_bind_context.m_index_buffer == ibo);
			device_context()->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);
            s_bind_context.m_index_buffer = nullptr;
        }
	}
	
	D3D11_MAP get_mapping_type_map_buff_range(MappingType type)
	{
		/*
		* GL_INVALID_OPERATION is generated for any of the following conditions: 
		* [...]
		* GL_MAP_READ_BIT is set and any of GL_MAP_INVALIDATE_RANGE_BIT, GL_MAP_INVALIDATE_BUFFER_BIT or GL_MAP_UNSYNCHRONIZED_BIT is set. 
		* [...]
		*/
		switch (type)
		{
		default:
		case MAP_WRITE_AND_READ: return D3D11_MAP_READ_WRITE; break;
		case MAP_WRITE: return D3D11_MAP_WRITE_DISCARD;
		case MAP_READ:  return D3D11_MAP_READ;
		}
	}

	std::vector<unsigned char> ContextDX11::copy_buffer_CB(const ConstBuffer* cb)
	{
		D3D11_BUFFER_DESC sbd = {};
		sbd.Usage = D3D11_USAGE_STAGING;
		sbd.ByteWidth = cb->get_size();
		sbd.BindFlags = 0;
		sbd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		ID3D11Buffer* sb = nullptr;

		if (dx_op_success(device()->CreateBuffer(&sbd, nullptr, &sb)))
		{
			std::vector<unsigned char> result;
			device_context()->CopyResource(sb, *cb);
			D3D11_MAPPED_SUBRESOURCE mapping;

			if (dx_op_success(device_context()->Map(sb, 0, D3D11_MAP_READ, 0, &mapping)))
			{
				auto data_start_ptr = static_cast<unsigned char*>(mapping.pData);
				auto data_end_ptr = data_start_ptr + cb->get_size();
				result = std::vector<unsigned char>{ data_start_ptr, data_end_ptr };
				device_context()->Unmap(sb, 0);
			}

			sb->Release();
			return result;
		}

		return {};
	}

	std::vector<unsigned char> ContextDX11::copy_buffer_VBO(const VertexBuffer* vbo)
	{
		D3D11_BUFFER_DESC sbd = {};
		sbd.Usage = D3D11_USAGE_STAGING;
		sbd.ByteWidth = vbo->get_size();
		sbd.BindFlags = 0;
		sbd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		ID3D11Buffer* sb = nullptr;

		if (dx_op_success(device()->CreateBuffer(&sbd, nullptr, &sb)))
		{
			std::vector<unsigned char> result;
			device_context()->CopyResource(sb, *vbo);
			D3D11_MAPPED_SUBRESOURCE mapping;

			if (dx_op_success(device_context()->Map(sb, 0, D3D11_MAP_READ, 0, &mapping)))
			{
				auto data_start_ptr = static_cast<unsigned char*>(mapping.pData);
				auto data_end_ptr = data_start_ptr + vbo->get_size();
				result = std::move(std::vector<unsigned char>{ data_start_ptr, data_end_ptr });
				device_context()->Unmap(sb, 0);
			}

			sb->Release();
			return result;
		}
		return {};
	}

	std::vector<unsigned char> ContextDX11::copy_buffer_IBO(const IndexBuffer* ibo)
	{
		D3D11_BUFFER_DESC sbd = {};
		sbd.Usage = D3D11_USAGE_STAGING;
		sbd.ByteWidth = ibo->get_size();
		sbd.BindFlags = 0;
		sbd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		ID3D11Buffer* sb = nullptr;

		if (dx_op_success(device()->CreateBuffer(&sbd, nullptr, &sb)))
		{
			std::vector<unsigned char> result;
			device_context()->CopyResource(sb, *ibo);
			D3D11_MAPPED_SUBRESOURCE mapping;

			if (dx_op_success(device_context()->Map(sb, 0, D3D11_MAP_READ, 0, &mapping)))
			{
				auto data_start_ptr = static_cast<unsigned char*>(mapping.pData);
				auto data_end_ptr = data_start_ptr + ibo->get_size();
				result = std::vector<unsigned char>{ data_start_ptr, data_end_ptr };
				device_context()->Unmap(sb, 0);
			}

			sb->Release();
			return result;
		}

		return {};
	}

	unsigned char* ContextDX11::map_CB(ConstBuffer* cb, size_t start, size_t n, MappingType type)
	{
		D3D11_MAPPED_SUBRESOURCE source;
		dx_op_success(device_context()->Map(cb->m_buffer, 0, get_mapping_type_map_buff_range(type), 0, &source));
		//get ptr
		unsigned char* ptr = (unsigned char*)source.pData;
		//add offset
		ptr += start;
		//return
		return ptr;
	}
	
	unsigned char* ContextDX11::map_CB(ConstBuffer* cb, MappingType type)
	{
		D3D11_MAPPED_SUBRESOURCE source;
		dx_op_success(device_context()->Map(cb->m_buffer, 0, get_mapping_type_map_buff_range(type), 0, &source));
		//get ptr
		unsigned char* ptr = (unsigned char*)source.pData;
		//return
		return ptr;
	}

	void ContextDX11::unmap_CB(ConstBuffer* cb)
	{
		device_context()->Unmap(cb->m_buffer, 0);
	}
		
	unsigned char* ContextDX11::map_VBO(VertexBuffer* vbo, size_t start, size_t n, MappingType type)
	{
		D3D11_MAPPED_SUBRESOURCE source;
		dx_op_success(device_context()->Map(vbo->m_buffer, 0, get_mapping_type_map_buff_range(type), 0, &source));
		//get ptr
		unsigned char* ptr = (unsigned char*)source.pData;
		//add offset
		ptr += start;
		//return
		return ptr;
	}

	unsigned char* ContextDX11::map_VBO(VertexBuffer* vbo, MappingType type)
	{
		D3D11_MAPPED_SUBRESOURCE source;
		dx_op_success(device_context()->Map(vbo->m_buffer, 0, get_mapping_type_map_buff_range(type), 0, &source));
		//get ptr
		unsigned char* ptr = (unsigned char*)source.pData;
		//return
		return ptr;
	}

	void ContextDX11::unmap_VBO(VertexBuffer* vbo)
	{
		device_context()->Unmap(vbo->m_buffer, 0);
	}

	unsigned int* ContextDX11::map_IBO(IndexBuffer* ibo, size_t start, size_t n, MappingType type)
	{
		D3D11_MAPPED_SUBRESOURCE source;
		dx_op_success(device_context()->Map(ibo->m_buffer, 0, get_mapping_type_map_buff_range(type), 0, &source));
		//get ptr
		unsigned char* ptr = (unsigned char*)source.pData;
		//add offset
		ptr += start * sizeof(unsigned int);
		//return
		return (unsigned int*)ptr;
	}

	unsigned int* ContextDX11::map_IBO(IndexBuffer* ibo, MappingType type)
	{
		D3D11_MAPPED_SUBRESOURCE source;
		dx_op_success(device_context()->Map(ibo->m_buffer, 0, get_mapping_type_map_buff_range(type), 0, &source));
		//get ptr
		unsigned char* ptr = (unsigned char*)source.pData;
		//return
		return (unsigned int*)ptr;
	}

	void ContextDX11::unmap_IBO(IndexBuffer* ibo)
	{
		device_context()->Unmap(ibo->m_buffer, 0);
	}

	unsigned char* ContextDX11::map_TBO(Texture* tbo, MappingType type)
	{
		if (tbo && s_bind_context.m_texture_buffer != tbo)
		{
			//unbind / unmap
			if (s_bind_context.m_texture_buffer)
			{
				unmap_TBO(s_bind_context.m_texture_buffer);
			}
			//update
			s_bind_context.m_texture_buffer = tbo;
			//bind
			D3D11_MAPPED_SUBRESOURCE source;
			switch (tbo->m_type)
			{
			case DX_TEXTURE_1D: device_context()->Map((ID3D11Texture1D*)tbo, 0, get_mapping_type_map_buff_range(type), 0, &source);
			case DX_TEXTURE_2D: device_context()->Map((ID3D11Texture2D*)tbo, 0, get_mapping_type_map_buff_range(type), 0, &source);
			case DX_TEXTURE_3D: device_context()->Map((ID3D11Texture3D*)tbo, 0, get_mapping_type_map_buff_range(type), 0, &source);
			default:break;
			}
			//map
			return (unsigned char*)source.pData;;
		}
        return nullptr;
	}

	void ContextDX11::unmap_TBO(Texture* tbo)
	{
		if (tbo)
		{
			square_assert(s_bind_context.m_texture_buffer == tbo);
			//unmap
			switch (tbo->m_type)
			{
			case DX_TEXTURE_1D: device_context()->Unmap((ID3D11Texture1D*)tbo, 0);
			case DX_TEXTURE_2D: device_context()->Unmap((ID3D11Texture2D*)tbo, 0);
			case DX_TEXTURE_3D: device_context()->Unmap((ID3D11Texture3D*)tbo, 0);
			default:break;
			}
			//update
			s_bind_context.m_texture_buffer = nullptr;
		}
	}

	void ContextDX11::delete_CB(ConstBuffer*& cb)
	{
		//test
		if (s_bind_context.m_const_buffer == cb)
		{
			unbind_CB(s_bind_context.m_const_buffer);
		}
		//safe delete
		delete cb;
		cb = nullptr;
	}

	void ContextDX11::delete_VBO(VertexBuffer*& vbo)
	{
        //test
        if(s_bind_context.m_vertex_buffer == vbo)
        {
            unbind_VBO(s_bind_context.m_vertex_buffer);
        }
        //safe delete
		delete vbo;
		vbo = nullptr;
	}

	void ContextDX11::delete_IBO(IndexBuffer*& ibo)
    {
        //test
        if(s_bind_context.m_index_buffer == ibo)
        {
            unbind_IBO(s_bind_context.m_index_buffer);
        }
        //safe delete
		delete ibo;
		ibo = nullptr;
	}
	/*
		Draw
	*/
	inline static D3D10_PRIMITIVE_TOPOLOGY get_draw_type(DrawType type)
	{
		switch (type)
		{
		case DRAW_POINTS:         return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;  break;
		case DRAW_LINES:          return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;  break;
		case DRAW_LINE_LOOP:      return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;  break;
		case DRAW_TRIANGLES:      return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST; break;
		case DRAW_TRIANGLE_STRIP: return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;  break;
		default:                   return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED; break;
		}
	}

	void ContextDX11::draw_arrays(DrawType type, unsigned int n)
	{
		//shader unbind
		if(s_bind_context.m_shader)
			s_bind_context.m_shader->bind_global_buffer(this);
		device_context()->IASetPrimitiveTopology(get_draw_type(type));
		device_context()->Draw(n, 0);
	}

	void ContextDX11::draw_arrays(DrawType type, unsigned int start, unsigned int size)
	{
		if (s_bind_context.m_shader)
			s_bind_context.m_shader->bind_global_buffer(this);
		device_context()->IASetPrimitiveTopology(get_draw_type(type));
		device_context()->Draw(size, start);
	}

	void ContextDX11::draw_elements(DrawType type, unsigned int n)
	{
		if (s_bind_context.m_shader)
			s_bind_context.m_shader->bind_global_buffer(this);
		device_context()->IASetPrimitiveTopology(get_draw_type(type));
		device_context()->DrawIndexed(n, 0, 0);
	}

	void ContextDX11::draw_elements(DrawType type, unsigned int start, unsigned int n)
	{
		if (s_bind_context.m_shader)
			s_bind_context.m_shader->bind_global_buffer(this);
		device_context()->IASetPrimitiveTopology(get_draw_type(type));
		device_context()->DrawIndexed(n, start, 0);
	}
	/*
		InputLayout
	*/
	const char* attribute_type_to_semantic_name(AttributeType type, int& semantic_index)
	{
		switch (type)
		{
		//case ATT_POSITION:  semantic_index = 0;  return "POSITION";
		case ATT_POSITION0: semantic_index = 0;  return "POSITION";
		case ATT_NORMAL0:   semantic_index = 0;  return "NORMAL";
		case ATT_TEXCOORD0: semantic_index = 0;  return "TEXCOORD";
		case ATT_TANGENT0:  semantic_index = 0;  return "TANGENT";
		case ATT_BINORMAL0: semantic_index = 0;  return "BINORMAL";
		case ATT_COLOR0:    semantic_index = 0;  return "COLOR";

		case ATT_POSITION1: semantic_index = 1;  return "POSITION";
		case ATT_NORMAL1:   semantic_index = 1;  return "NORMAL";
		case ATT_TEXCOORD1: semantic_index = 1;  return "TEXCOORD";
		case ATT_TANGENT1:  semantic_index = 1;  return "TANGENT";
		case ATT_BINORMAL1: semantic_index = 1;  return "BINORMAL";
		case ATT_COLOR1:    semantic_index = 1;  return "COLOR";

		case ATT_NORMAL2:   semantic_index = 2;  return "NORMAL";
		case ATT_TEXCOORD2: semantic_index = 2;  return "TEXCOORD";
		case ATT_TANGENT2:  semantic_index = 2;  return "TANGENT";
		case ATT_BINORMAL2: semantic_index = 2;  return "BINORMAL";
		case ATT_COLOR2:    semantic_index = 2;  return "COLOR";
		default: return "";
		}

	}
	DXGI_FORMAT attribute_strip_type_to_input_element(AttributeStripType type)
	{
		switch (type)
		{
		case AST_FLOAT: return DXGI_FORMAT_R32_FLOAT;
		case AST_FLOAT2: return DXGI_FORMAT_R32G32_FLOAT;
		case AST_FLOAT3: return DXGI_FORMAT_R32G32B32_FLOAT;
		case AST_FLOAT4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case AST_INT:    return DXGI_FORMAT_R32_SINT;
		case AST_INT2:   return DXGI_FORMAT_R32G32_SINT;
		case AST_INT3:   return DXGI_FORMAT_R32G32B32_SINT;
		case AST_INT4:   return DXGI_FORMAT_R32G32B32A32_SINT;
		case AST_UINT:   return DXGI_FORMAT_R32_UINT;
		case AST_UINT2:  return DXGI_FORMAT_R32G32_UINT;
		case AST_UINT3:  return DXGI_FORMAT_R32G32B32_UINT;
		case AST_UINT4:  return DXGI_FORMAT_R32G32B32A32_UINT;
		case AST_TLESS:  return DXGI_FORMAT_R32_TYPELESS;
		case AST_TLESS2: return DXGI_FORMAT_R32G32_TYPELESS;
		case AST_TLESS3: return DXGI_FORMAT_R32G32B32_TYPELESS;
		case AST_TLESS4: return DXGI_FORMAT_R32G32B32A32_TYPELESS;
		default:         return DXGI_FORMAT_UNKNOWN; 
		}
	}


	InputLayout::InputLayout
	(
		  const AttributeList& attributes
		, const std::vector< D3D11_INPUT_ELEMENT_DESC >& description
	)
	: m_attributes(attributes)
	, m_description(description)
	{
	}

	static std::map<std::string, DXGI_FORMAT> reflaction_input_layout(ID3DBlob* shader)
	{
		//////////////////////////////////////
		std::map<std::string, DXGI_FORMAT> semantic_type_map;
		//////////////////////////////////////
		ID3D11ShaderReflection* shader_ref = nullptr;
		if (FAILED(D3DReflect(
			 shader->GetBufferPointer()
		   , shader->GetBufferSize()
		   , IID_ID3D11ShaderReflection
		   , (void**)&shader_ref
		)))
		{
			return semantic_type_map;
		}
		//////////////////////////////////////
		// Get shader info
		D3D11_SHADER_DESC shaderDesc;
		shader_ref->GetDesc(&shaderDesc);

		// Read input layout description from shader info
		for (uint32 i = 0; i< shaderDesc.InputParameters; i++)
		{
			D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
			shader_ref->GetInputParameterDesc(i, &paramDesc);
			DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_TYPELESS;

			// determine DXGI format
			if (paramDesc.Mask == 1)
			{
				     if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) format = DXGI_FORMAT_R32_UINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) format = DXGI_FORMAT_R32_SINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)format = DXGI_FORMAT_R32_FLOAT;
			}
			else if (paramDesc.Mask <= 3)
			{
				     if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) format = DXGI_FORMAT_R32G32_UINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) format = DXGI_FORMAT_R32G32_SINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)format = DXGI_FORMAT_R32G32_FLOAT;
			}
			else if (paramDesc.Mask <= 7)
			{
				     if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) format = DXGI_FORMAT_R32G32B32_UINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) format = DXGI_FORMAT_R32G32B32_SINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)format = DXGI_FORMAT_R32G32B32_FLOAT;
			}
			else if (paramDesc.Mask <= 15)
			{
				     if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) format = DXGI_FORMAT_R32G32B32A32_UINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) format = DXGI_FORMAT_R32G32B32A32_SINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			}

			//save element desc
			semantic_type_map[std::string(paramDesc.SemanticName)] = format;
		}
		return std::move(semantic_type_map);
	}

	static bool test_input_layout(ID3DBlob* shader, const std::vector<D3D11_INPUT_ELEMENT_DESC>& vals)
	{
		auto map = reflaction_input_layout(shader);
		for (auto it : map)
		{
			bool find = false;
			for (auto val : vals)
			{
				if (it.first == std::string(val.SemanticName))
				{
					if (it.second != val.Format)
					{
						return false;
					}
					find = true;
					break;
				}
			}
			if (!find) return false;
		}
		return true;
	}
	
	ID3D11InputLayout* InputLayout::build(ContextDX11* context11, ID3DBlob* shader)
	{
		if(!shader) return nullptr;
		//input layout d3d11
		ID3D11InputLayout* vertex_layout{ nullptr };
		//debug
#ifdef _DEBUG
		//test
		if (!test_input_layout(shader, m_description)) return nullptr;
#endif
		//SUCCEEDED
		if (context11->dx_op_success(context11->device()->CreateInputLayout(
			  m_description.data()
			, m_description.size()
			, shader->GetBufferPointer()
			, shader->GetBufferSize()
			, &vertex_layout
		)))
		{
			return vertex_layout;
		}
		return nullptr;
	}

	ID3D11InputLayout* InputLayout::get_dx11_input_layout(ContextDX11* context11, Shader* shader)
	{
		//find
		auto it = m_map_layouts.find(shader);
		//ok if find
		if(it != m_map_layouts.end()) return it->second;
		//build
		auto dx11il = build(context11, shader->binary(ST_VERTEX_SHADER));
		//if null
		if (!dx11il) return nullptr;
		//else save
		m_map_layouts[shader] = dx11il;
		//end return
		return dx11il;
	}

	InputLayout::~InputLayout()
	{
		for(auto s_il_pair : m_map_layouts)
			if(s_il_pair.second) 
				s_il_pair.second->Release();
	}

	InputLayout* ContextDX11::create_IL(const AttributeList& atl)
	{
		//directx li
		std::vector<D3D11_INPUT_ELEMENT_DESC> layouts;
		//for each  attribute
		size_t i = 0;
		//create attributes
		for (const auto& at : atl)
		{
			D3D11_INPUT_ELEMENT_DESC layout;
			int semantic_index = 0;
			layout.SemanticName         = attribute_type_to_semantic_name(at.m_attribute, semantic_index);
			layout.SemanticIndex        = semantic_index;
			layout.Format		        = attribute_strip_type_to_input_element(at.m_strip);
			layout.InputSlot            = 0;
			layout.AlignedByteOffset    = at.m_offset;
			layout.InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
			layout.InstanceDataStepRate = 0;
			layouts.push_back(layout);
		}
		//success
		return new InputLayout(atl, layouts);
	}

	size_t ContextDX11::size_IL(const InputLayout* layout)
	{
		return layout->m_attributes.size();
	}

	bool ContextDX11::has_a_position_IL(const InputLayout* layout)
	{
		for (auto& in : layout->m_attributes)
			if (in.is_position_transform())
				return true;
		return false;
	}

	size_t ContextDX11::position_offset_IL(const InputLayout* layout)
	{
		for (auto& in : layout->m_attributes)
			if (in.is_position_transform())
				return in.m_offset;
		return ~((size_t)0);
	}

	void ContextDX11::bind_IL(InputLayout* layout)
	{
        if(layout && s_bind_context.m_input_layout != layout)
        {
            //bind?
            if(s_bind_context.m_input_layout)
            {
                unbind_IL(s_bind_context.m_input_layout);
            }
			//get
			ID3D11InputLayout* dx11il = layout->get_dx11_input_layout(this, s_bind_context.m_shader);
			//test
			if (!dx11il)
			{
				m_errors.push_back({ "D3D11 Wrapper, bind_IL: can not build a input layout" });
				return;
			}
            //bind
			device_context()->IASetInputLayout(dx11il);
            //save
            s_bind_context.m_input_layout = layout;
        }
	}

	void ContextDX11::unbind_IL(InputLayout* layout)
	{
        if(layout)
        {
            //test
            square_assert(s_bind_context.m_input_layout==layout);
            //unbind
			device_context()->IASetInputLayout(nullptr);
            //safe
            s_bind_context.m_input_layout = nullptr;
        }
	}

	void ContextDX11::delete_IL(InputLayout*& il)
	{
		delete  il;
		il = nullptr;
	}

	//RGBA
	Vec4 ContextDX11::get_color(const Vec2& pixel) const
	{
		//create box
		D3D11_BOX box;
		box.left = pixel.x;
		box.right = box.left + 1;
		box.top = pixel.y;
		box.bottom = box.top + 1;
		box.front = 0;
		box.back = 1;
		//get image
		ID3D11Texture2D* view_buffer;
		if (!SUCCEEDED(swap()->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&view_buffer))) return {};
		//copy to query buffer
		device_context()->CopySubresourceRegion(m_query_buffer[0]->m_texture2D, 0, 0, 0, 0, view_buffer, 0, &box);
		//mapped
		D3D11_MAPPED_SUBRESOURCE source;
		device_context()->Map((ID3D11Texture2D*)m_query_buffer[0], 0, D3D11_MAP_READ, 0, &source);
		unsigned char* color = (unsigned char*)source.pData;
		Vec4 color_normalize(color[0] / 255.0f, color[1] / 255.0f, color[3] / 255.0f, color[4] / 255.0f);
		device_context()->Unmap((ID3D11Texture2D*)m_query_buffer[0], 0);
		//ok
		return color_normalize;
	}

	//DEPTH
	float ContextDX11::get_depth(const Vec2& pixel) const
	{	
		//if...
		if (!m_view_target) return 0;
		if (!m_view_target->m_depth_texture) return 0;
		//create box
		D3D11_BOX box;
		box.left = pixel.x;
		box.right = box.left + 1;
		box.top = pixel.y;
		box.bottom = box.top + 1;
		box.front = 0;
		box.back = 1;
		//copy to query buffer
		device_context()->CopySubresourceRegion(m_query_buffer[1]->m_texture2D, 0, 0, 0, 0, m_view_target->m_depth_texture, 0, &box);
		//mapped
		D3D11_MAPPED_SUBRESOURCE source;
		device_context()->Map((ID3D11Texture2D*)m_query_buffer[1], 0, D3D11_MAP_READ, 0, &source);
		float depth = *((float*)source.pData);
		device_context()->Unmap((ID3D11Texture2D*)m_query_buffer[1], 0);
		//ok
		return depth;
	}

	/*
		Textures
	*/
	inline const DXGI_FORMAT get_texture_format(TextureFormat type)
	{
		//select
		switch (type)
		{
		//RGBA
		case TF_RGBA8: return DXGI_FORMAT_R8G8B8A8_UNORM; break;
		//uint
		case TF_RGBA16UI: return DXGI_FORMAT_R16G16B16A16_UINT; break;
		case TF_RGBA32UI: return DXGI_FORMAT_R32G32B32A32_UINT; break;
		//int
		case TF_RGBA16I: return DXGI_FORMAT_R16G16B16A16_SINT; break;
		case TF_RGBA32I: return DXGI_FORMAT_R32G32B32A32_SINT; break;
		//float
		case TF_RGBA16F: return DXGI_FORMAT_R16G16B16A16_FLOAT; break;
		case TF_RGBA32F: return DXGI_FORMAT_R32G32B32A32_FLOAT; break;
		////////////////////
		//RGB
		case TF_RGB8: return DXGI_FORMAT_R8G8B8A8_UNORM; /*add_alpha = true;*/ break;
		//uint
		case TF_RGB16UI: return DXGI_FORMAT_R16G16B16A16_UINT; /*add_alpha = true;*/ break;
		case TF_RGB32UI: return DXGI_FORMAT_R32G32B32A32_UINT; /*add_alpha = true;*/ break;
		//int
		case TF_RGB16I: return DXGI_FORMAT_R16G16B16A16_SINT;  /*add_alpha = true;*/ break;
		case TF_RGB32I: return DXGI_FORMAT_R32G32B32A32_SINT;  /*add_alpha = true;*/ break;
		//float
		case TF_RGB16F: return DXGI_FORMAT_R16G16B16A16_FLOAT; /*add_alpha = true;*/ break;
		case TF_RGB32F: return DXGI_FORMAT_R32G32B32A32_FLOAT; /*add_alpha = true;*/ break;
		////////////////////
		//RG
		case TF_RG8: return DXGI_FORMAT_R8G8_UNORM; break;
		//uint
		case TF_RG16UI: return DXGI_FORMAT_R16G16_UINT; break;
		case TF_RG32UI: return DXGI_FORMAT_R32G32_UINT; break;
		//int
		case TF_RG16I: return DXGI_FORMAT_R16G16_SINT; break;
		case TF_RG32I: return DXGI_FORMAT_R32G32_SINT; break;
		//float
		case TF_RG16F: return DXGI_FORMAT_R16G16_FLOAT; break;
		case TF_RG32F: return DXGI_FORMAT_R32G32_FLOAT; break;
		////////////////////
		//R
		case TF_R8: return DXGI_FORMAT_R8_UNORM; break;
		//uint
		case TF_R16UI: return DXGI_FORMAT_R16_UINT; break;
		case TF_R32UI: return DXGI_FORMAT_R32_UINT; break;
		//int
		case TF_R16I: return DXGI_FORMAT_R16_SINT; break;
		case TF_R32I: return DXGI_FORMAT_R32_SINT; break;
		//float
		case TF_R16F: return DXGI_FORMAT_R16_FLOAT; break;
		case TF_R32F: return DXGI_FORMAT_R32_FLOAT; break;
		///////////////////
		case TF_RGB565:  return DXGI_FORMAT_B5G6R5_UNORM;    /*swap_r_b = true;*/ break;
		case TF_RGB5A1:  return DXGI_FORMAT_B5G5R5A1_UNORM;  /*swap_r_b = true;*/ break;
		case TF_RGBA4:   return DXGI_FORMAT_B4G4R4A4_UNORM;  /*swap_r_b = true;*/  break;
		///////////////////
		//DEPTH & STENCIL
		//case TF_DEPTH16_STENCIL8: return DXGI_FORMAT_D16_UNORM_S8_UINT; break;
		case TF_DEPTH24_STENCIL8: return DXGI_FORMAT_D24_UNORM_S8_UINT; break;
		case TF_DEPTH32_STENCIL8: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT; break;
		//DEPTH
		case TF_DEPTH_COMPONENT16: return DXGI_FORMAT_D16_UNORM; break;
		//case TF_DEPTH_COMPONENT24: return GL_DEPTH_COMPONENT24; break;
		case TF_DEPTH_COMPONENT32: return DXGI_FORMAT_D32_FLOAT; break;

		default: return DXGI_FORMAT_FORCE_UINT; break;
		}
	}
	
	inline const UINT get_textut_pixel_size(TextureFormat type)
	{
		switch (type)
		{
		case Square::Render::TF_RGBA8:		return 1 * 4;

		case Square::Render::TF_RGBA16UI:
		case Square::Render::TF_RGBA16I:
		case Square::Render::TF_RGBA16F:    return 2 * 4;

		case Square::Render::TF_RGBA32UI:
		case Square::Render::TF_RGBA32I:
		case Square::Render::TF_RGBA32F:    return 4 * 4;

		case Square::Render::TF_RGB8:		return 1 * 3;

		case Square::Render::TF_RGB16UI:
		case Square::Render::TF_RGB16I:
		case Square::Render::TF_RGB16F:		return 2 * 3;

		case Square::Render::TF_RGB32UI:
		case Square::Render::TF_RGB32I:
		case Square::Render::TF_RGB32F:		return 4 * 3;

		case Square::Render::TF_RG8:		return 1 * 2;

		case Square::Render::TF_RG16UI:
		case Square::Render::TF_RG16I:
		case Square::Render::TF_RG16F:		return 2 * 2;

		case Square::Render::TF_RG32UI:
		case Square::Render::TF_RG32I:
		case Square::Render::TF_RG32F:		return 4 * 2;

		case Square::Render::TF_R8:		    return 1 * 1;

		case Square::Render::TF_R16UI:
		case Square::Render::TF_R16I:
		case Square::Render::TF_R16F:		return 2 * 1;

		case Square::Render::TF_R32UI:
		case Square::Render::TF_R32I:
		case Square::Render::TF_R32F:		return 4 * 1;

		case Square::Render::TF_RGB565:		
		case Square::Render::TF_RGB5A1:
		case Square::Render::TF_RGBA4:		return 2;

		case Square::Render::TF_DEPTH16_STENCIL8: return 3;
		case Square::Render::TF_DEPTH24_STENCIL8: return 4;
		case Square::Render::TF_DEPTH32_STENCIL8: return 5;

		case Square::Render::TF_DEPTH_COMPONENT16: return 2;
		case Square::Render::TF_DEPTH_COMPONENT24: return 3;
		case Square::Render::TF_DEPTH_COMPONENT32: return 4;
		default:								   return 0;
		}
	}

	static D3D11_FILTER get_texture_min_filter(TextureMinFilterType min, TextureMagFilterType mag)
	{
		switch (min)
		{
		default:
		case TMIN_NEAREST:                return mag == TMAG_NEAREST ? D3D11_FILTER_MIN_MAG_MIP_POINT 
																	 : D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
		case TMIN_NEAREST_MIPMAP_NEAREST: return mag == TMAG_NEAREST ? D3D11_FILTER_MIN_MAG_MIP_POINT
																	 : D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
		case TMIN_NEAREST_MIPMAP_LINEAR:  return mag == TMAG_NEAREST ? D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR
																	 : D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
		case TMIN_LINEAR:                 return mag == TMAG_NEAREST ? D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR
																	 : D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		case TMIN_LINEAR_MIPMAP_NEAREST: return mag == TMAG_NEAREST ? D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT
																    : D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		case TMIN_LINEAR_MIPMAP_LINEAR:  return mag == TMAG_NEAREST ? D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR
																    : D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		}
	}

	static D3D11_TEXTURE_ADDRESS_MODE get_texture_edge_type(TextureEdgeType type)
	{
		switch (type)
		{
		default:
		case TEDGE_CLAMP:  return D3D11_TEXTURE_ADDRESS_CLAMP;
		case TEDGE_REPEAT: return D3D11_TEXTURE_ADDRESS_WRAP;
		}
	}

	static const unsigned char* add_alpha_to_image(Allocator* allocator, const unsigned char* rgb, size_t w, size_t h, size_t t_size,void* value_alpha)
	{
		unsigned char* rgba = (unsigned char*)allocator->alloc(w*h*(4*t_size), "static Square::Render::add_alpha_to_image", AllocType::ALCT_DEFAULT);
		for (size_t y = 0; y != w; ++y)
		for (size_t x = 0; x != w; ++x)
		{
			const unsigned char* pixel_rgb  = &rgb[(x + (y*w))*(3 * t_size)];
			unsigned char* pixel_rgba = &rgba[(x + (y*w))*(4 * t_size)];
			//copy pixel
			std::memcpy(pixel_rgba, pixel_rgb, (3 * t_size));
			//copy alpha
			std::memcpy(pixel_rgba + (3 * t_size), value_alpha, (1 * t_size));
		}
		return rgba;
	}

	static const unsigned char* add_alpha_if_need(Allocator* allocator, const unsigned char* data, size_t w, size_t h, TextureFormat type)
	{
		//select
		switch (type)
		{
		////////////////////////////////////////////////////////////////////////////////
		//RGB
		case TF_RGB8:
		{
			unsigned char value = 0xFF;
			return add_alpha_to_image(allocator, data, w, h, sizeof(value), &value);
		}
		break;
		//uint
		case TF_RGB16UI:
		{
			unsigned __int16 value = 0xFFFF;
			return add_alpha_to_image(allocator, data, w, h, sizeof(value), &value);
		}
		break; 
		case TF_RGB32UI:
		{
			unsigned __int32 value = 0xFFFFFFF;
			return add_alpha_to_image(allocator, data, w, h, sizeof(value), &value);
		}
		break;
		//int
		case TF_RGB16I:
		{
			__int16 value = 0x7FFF;
			return add_alpha_to_image(allocator, data, w, h, sizeof(value), &value);
		}
		break;
		case TF_RGB32I:
		{
			__int32 value = 0x7FFFFFFF;
			return add_alpha_to_image(allocator, data, w, h, sizeof(value), &value);
		}
		break;
		//float
		case TF_RGB16F: 
			//value struct
			union
			{
				uint16_t bits;			// all bits
				struct
				{
					uint16_t frac : 10;	// mantissa
					uint16_t exp  : 5;	// exponent
					uint16_t sign : 1;	// sign
				} 
				IEEE;
			}
			value;
			value.IEEE.exp  = 31-1;
			value.IEEE.frac = ~0;
			value.IEEE.sign = 0x0;
			return add_alpha_to_image(allocator, data, w, h, sizeof(value), &value);
		break;
		case TF_RGB32F:
		{
			float value = 340282346638528859811704183484516925440.0;
			return add_alpha_to_image(allocator, data, w, h, sizeof(value), &value);
		}
		break;
		////////////////////////////////////////////////////////////////////////////////
		default: break;
		}
		return nullptr;
	}
	
	static bool is_depth_texture(DXGI_FORMAT format)
	{
		switch (format)
		{
		default: return false;
		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_D32_FLOAT:
		return true;
		}
	}

	static DXGI_FORMAT texture_depth_to_typeless_texture(DXGI_FORMAT format)
	{
		DXGI_FORMAT new_format = format;
		     if (format == DXGI_FORMAT_D16_UNORM)         new_format = DXGI_FORMAT_R16_TYPELESS;
		else if (format == DXGI_FORMAT_D24_UNORM_S8_UINT) new_format = DXGI_FORMAT_R24G8_TYPELESS;
		else if (format == DXGI_FORMAT_D32_FLOAT)         new_format = DXGI_FORMAT_R32_TYPELESS;
		return new_format;
	}

	static DXGI_FORMAT texture_depth_to_resource_format(DXGI_FORMAT format)
	{
		DXGI_FORMAT new_format = format;
		     if (format == DXGI_FORMAT_D16_UNORM)         new_format = DXGI_FORMAT_R16_UNORM;
		else if (format == DXGI_FORMAT_D24_UNORM_S8_UINT) new_format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		else if (format == DXGI_FORMAT_D32_FLOAT)         new_format = DXGI_FORMAT_R32_FLOAT;
		return new_format;
	}

	Texture* ContextDX11::create_texture
	(
		const TextureRawDataInformation& data,
		const TextureGpuDataInformation& info
	)
	{
		const unsigned char* texture_bytes = data.m_bytes; 
		UINT pixel_size = get_textut_pixel_size(data.m_format);
		D3D11_USAGE usage = info.m_read_from_cpu ? D3D11_USAGE_STAGING : D3D11_USAGE_DEFAULT;
		UINT cpu_access_flags = info.m_read_from_cpu ? D3D11_CPU_ACCESS_READ : 0;
		UINT mip_levels = info.m_build_mipmap ? 0 : info.m_mipmap_max;
		UINT misc_flags = info.m_build_mipmap ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;
		UINT bind_flags = D3D11_BIND_SHADER_RESOURCE;
		//format
		DXGI_FORMAT texture_format_raw  = get_texture_format(data.m_format);
		bool	    depth_target        = is_depth_texture(texture_format_raw);
		DXGI_FORMAT texture_format_data = texture_depth_to_typeless_texture(texture_format_raw);
		DXGI_FORMAT texture_format_resource = texture_depth_to_resource_format(texture_format_raw);
		//render target
		if (depth_target) bind_flags |= D3D11_BIND_DEPTH_STENCIL;
		else              bind_flags |= D3D11_BIND_RENDER_TARGET;
		//new image if need
		auto new_image = Unique<const unsigned char[]> (add_alpha_if_need(allocator(), data.m_bytes, data.m_width, data.m_height, data.m_format), DefaultDelete(allocator()));
		//if new image alloc, point to new image
		if (new_image.get())
		{
			texture_bytes = new_image.get();
			pixel_size += 1; //alpha channel
		}
		//TEXTURE2D
		D3D11_TEXTURE2D_DESC texture_desc
		{
			data.m_width,		//UINT Width;
			data.m_height,		//UINT Height;
			mip_levels,			//UINT MipLevels;
			1,					//UINT ArraySize;
			texture_format_data,//DXGI_FORMAT Format;
			1, 0,				//DXGI_SAMPLE_DESC SampleDesc;
			usage,				//D3D11_USAGE Usage;
			bind_flags,			//UINT BindFlags;
			cpu_access_flags,	//UINT CPUAccessFlags;
			misc_flags	        //UINT MiscFlags;
		};
		//build
		ID3D11Texture2D* d11_texture;
		if (dx_op_success(device()->CreateTexture2D(&texture_desc, nullptr, &d11_texture)))
		{
			//upload
			if (texture_bytes)
			{
				device_context()->UpdateSubresource(d11_texture, 0, nullptr, texture_bytes, (data.m_width * pixel_size), 0);
			}
			//declare
			auto* texture2D = new Texture2D();
			texture2D->m_texture2D = d11_texture;			
			texture2D->m_width = data.m_width;
			texture2D->m_height = data.m_height;
			texture2D->m_is_depth = depth_target;
			texture2D->m_format_raw = texture_format_raw;
			texture2D->m_format_data = texture_format_data;
			texture2D->m_format_shader_resource = texture_format_resource;
			//view
			D3D11_SHADER_RESOURCE_VIEW_DESC s_resource_view = {};
			s_resource_view.Format = texture_format_resource;
			s_resource_view.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			s_resource_view.Texture2D.MostDetailedMip = 0;
			s_resource_view.Texture2D.MipLevels = info.m_build_mipmap ? -1 : info.m_mipmap_max;
			//try
			if (!dx_op_success(device()->CreateShaderResourceView(d11_texture, &s_resource_view, &texture2D->m_resource_view)))
			{
				m_errors.push_back({ "create_texture: wrong to generate shader resource view" });
				//wrong
				return texture2D;
			}
			//build
			if (info.m_build_mipmap)
			{
				device_context()->GenerateMips(texture2D->m_resource_view);
			}
			//build sampler
			D3D11_SAMPLER_DESC sampler_desc = {};
			sampler_desc.AddressU = get_texture_edge_type(info.m_edge_s);
			sampler_desc.AddressV = get_texture_edge_type(info.m_edge_t);
			sampler_desc.AddressW = get_texture_edge_type(info.m_edge_r);
			sampler_desc.Filter = get_texture_min_filter(info.m_min_type, info.m_mag_type);
			sampler_desc.BorderColor[0] = 0;
			sampler_desc.BorderColor[1] = 0;
			sampler_desc.BorderColor[2] = 0;
			sampler_desc.BorderColor[3] = 0;
			sampler_desc.MaxAnisotropy = (UINT)info.m_anisotropy;
			sampler_desc.MinLOD = (FLOAT)info.m_mipmap_min;
			sampler_desc.MaxLOD = (FLOAT)info.m_mipmap_max;
			//try
			if (!dx_op_success(device()->CreateSamplerState(&sampler_desc, &texture2D->m_sempler)))
			{
				//wrong
				m_errors.push_back({ "create_texture: wrong to generate texture sampler" });
			}
			//wrong
			return texture2D;
		}
		//test
		print_errors();
		//return texture
		return nullptr;
	}

	Texture* ContextDX11::create_cube_texture
	(
		const TextureRawDataInformation data[6],
		const TextureGpuDataInformation& info
	)
	{
		const unsigned char* texture_bytes[]
		{
			data[0].m_bytes, data[1].m_bytes, data[2].m_bytes,
			data[3].m_bytes, data[4].m_bytes, data[5].m_bytes
		};
		UINT pixel_size = get_textut_pixel_size(data[0].m_format);
		D3D11_USAGE usage = info.m_read_from_cpu ? D3D11_USAGE_STAGING : D3D11_USAGE_DEFAULT;
		UINT cpu_access_flags = info.m_read_from_cpu ? D3D11_CPU_ACCESS_READ : 0;
		UINT mip_levels = info.m_build_mipmap ? 0 : info.m_mipmap_max;
		UINT misc_flags = info.m_build_mipmap ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;
		UINT bind_flags = D3D11_BIND_SHADER_RESOURCE;
		//is a cubemap
		misc_flags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
		//format
		DXGI_FORMAT texture_format_raw = get_texture_format(data[0].m_format);
		bool	    depth_target = is_depth_texture(texture_format_raw);
		DXGI_FORMAT texture_format_data = texture_depth_to_typeless_texture(texture_format_raw);
		DXGI_FORMAT texture_format_resource = texture_depth_to_resource_format(texture_format_raw);
		//render target
		if (depth_target) bind_flags |= D3D11_BIND_DEPTH_STENCIL;
		else              bind_flags |= D3D11_BIND_RENDER_TARGET;
		//new image if need
		auto new_image0 = Unique<const unsigned char[]>(add_alpha_if_need(allocator(), data[0].m_bytes, data[0].m_width, data[0].m_height, data[0].m_format), DefaultDelete(allocator()));
		auto new_image1 = Unique<const unsigned char[]>(add_alpha_if_need(allocator(), data[1].m_bytes, data[1].m_width, data[1].m_height, data[1].m_format), DefaultDelete(allocator()));
		auto new_image2 = Unique<const unsigned char[]>(add_alpha_if_need(allocator(), data[2].m_bytes, data[2].m_width, data[2].m_height, data[2].m_format), DefaultDelete(allocator()));
		auto new_image3 = Unique<const unsigned char[]>(add_alpha_if_need(allocator(), data[3].m_bytes, data[3].m_width, data[3].m_height, data[3].m_format), DefaultDelete(allocator()));
		auto new_image4 = Unique<const unsigned char[]>(add_alpha_if_need(allocator(), data[4].m_bytes, data[4].m_width, data[4].m_height, data[4].m_format), DefaultDelete(allocator()));
		auto new_image5 = Unique<const unsigned char[]>(add_alpha_if_need(allocator(), data[5].m_bytes, data[5].m_width, data[5].m_height, data[5].m_format), DefaultDelete(allocator()));
		//if new image alloc, point to new image
		if (new_image0.get())
		{
			texture_bytes[0] = new_image0.get();
			texture_bytes[1] = new_image1.get();
			texture_bytes[2] = new_image2.get();
			texture_bytes[3] = new_image3.get();
			texture_bytes[4] = new_image4.get();
			texture_bytes[5] = new_image5.get();
			pixel_size += 1;
		}
		//TEXTURE2D
		D3D11_TEXTURE2D_DESC texture_desc
		{
			data[0].m_width,	 //UINT Width;
			data[0].m_height,	 //UINT Height;
			mip_levels,			 //UINT MipLevels;
			6,                   //UINT ArraySize;
			texture_format_data, //DXGI_FORMAT Format;
			1, 0,                //DXGI_SAMPLE_DESC SampleDesc;
			usage,               //D3D11_USAGE Usage;
			bind_flags,          //UINT BindFlags;
			cpu_access_flags,	 //UINT CPUAccessFlags;
			misc_flags			 //UINT MiscFlags;
		};	
		//build
		ID3D11Texture2D* d11_texture;
		if (dx_op_success(device()->CreateTexture2D(&texture_desc, nullptr, &d11_texture)))
		{
			//upload
			for (int i = 0; i != 6; ++i)
			{
				if(texture_bytes[i])
					device_context()->UpdateSubresource(d11_texture, 0, nullptr, texture_bytes[i], (data[i].m_width * pixel_size), 0);
			}
			//declare
			auto* texture2D = new Texture2D();
			texture2D->m_texture2D = d11_texture;
			texture2D->m_width = data[0].m_width;
			texture2D->m_height = data[0].m_height;
			texture2D->m_is_cube = true;
			texture2D->m_is_depth = depth_target;
			texture2D->m_format_raw = texture_format_raw;
			texture2D->m_format_data = texture_format_data;
			texture2D->m_format_shader_resource = texture_format_resource;
			//view
			D3D11_SHADER_RESOURCE_VIEW_DESC s_resource_view = {};
			s_resource_view.Format = texture_format_resource;
			s_resource_view.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
			s_resource_view.TextureCube.MostDetailedMip = 0;
			s_resource_view.TextureCube.MipLevels = info.m_build_mipmap ? -1 : info.m_mipmap_max;
			//try
			if (!SUCCEEDED(device()->CreateShaderResourceView(d11_texture, &s_resource_view, &texture2D->m_resource_view)))
			{
				m_errors.push_back({ "create_cube_texture: wrong to generate shader resource view" });
				//wrong
				return texture2D;
			}
			//build
			if (info.m_build_mipmap)
			{
				device_context()->GenerateMips(texture2D->m_resource_view);
			}
			//sampler
			D3D11_SAMPLER_DESC sampler_desc = {};
			sampler_desc.AddressU = get_texture_edge_type(info.m_edge_s);
			sampler_desc.AddressV = get_texture_edge_type(info.m_edge_t);
			sampler_desc.AddressW = get_texture_edge_type(info.m_edge_r);;
			sampler_desc.Filter = get_texture_min_filter(info.m_min_type, info.m_mag_type);
			sampler_desc.BorderColor[0] = 0;
			sampler_desc.BorderColor[1] = 0;
			sampler_desc.BorderColor[2] = 0;
			sampler_desc.BorderColor[3] = 0;
			sampler_desc.MaxAnisotropy = (UINT)info.m_anisotropy;
			sampler_desc.MinLOD = (FLOAT)info.m_mipmap_min;
			sampler_desc.MaxLOD = (FLOAT)info.m_mipmap_max;
			//try
			if (!SUCCEEDED(device()->CreateSamplerState(&sampler_desc, &texture2D->m_sempler)))
			{
				//wrong
				m_errors.push_back({ "create_cube_texture: wrong to generate texture sampler" });
			}
			//wrong
			return texture2D;
		}
		//test
		print_errors();
		//return texture
		return nullptr;
	}
        
	std::vector< unsigned char > ContextDX11::get_texture(Texture* tex, int level)
	{
		D3D11_MAPPED_SUBRESOURCE source;
		ID3D11Resource* texture_resource;
		switch (tex->m_type)
		{
		case DX_TEXTURE_1D: texture_resource = ((Texture1D*)tex)->m_texture1D; break;
		case DX_TEXTURE_2D: texture_resource = ((Texture2D*)tex)->m_texture2D; break;
		case DX_TEXTURE_3D: texture_resource = ((Texture3D*)tex)->m_texture3D; break;
		default: return {};
		}
		device_context()->Map(texture_resource, level, get_mapping_type_map_buff_range(MAP_READ), 0, &source);
		//size
		size_t size = source.RowPitch * (tex->m_height >> level);
		//alloc
		std::vector< unsigned char > output;
		output.resize(size);
		//copy
		std::memcpy(output.data(), source.pData, size);
		//end
		device_context()->Unmap(texture_resource, level);
		//return
		return output;
	}

	std::vector< unsigned char > ContextDX11::get_texture(Texture* tex, int cube, int level)
	{
		D3D11_MAPPED_SUBRESOURCE source;
		ID3D11Resource* texture_resource;
		switch (tex->m_type)
		{
		case DX_TEXTURE_1D: texture_resource = ((Texture1D*)tex)->m_texture1D; break;
		case DX_TEXTURE_2D: texture_resource = ((Texture2D*)tex)->m_texture2D; break;
		case DX_TEXTURE_3D: texture_resource = ((Texture3D*)tex)->m_texture3D; break;
		default: return {};
		}
		device_context()->Map(texture_resource, D3D11CalcSubresource(0, cube, level), get_mapping_type_map_buff_range(MAP_READ), 0, &source);
		//size
		size_t size = source.RowPitch * (tex->m_height >> level);
		//alloc
		std::vector< unsigned char > output;
		output.resize(size);
		//copy
		std::memcpy(output.data(), source.pData, size);
		//end
		device_context()->Unmap(texture_resource, level);
		//return
		return output;
	}

	void ContextDX11::bind_texture(Texture* ctx_texture, int texture_id, int sempler_id)
	{
        if (ctx_texture && ctx_texture != s_bind_context.m_textures[texture_id])
        {
            //enable
			device_context()->VSSetShaderResources(texture_id, 1, &ctx_texture->m_resource_view);
			device_context()->PSSetShaderResources(texture_id, 1, &ctx_texture->m_resource_view);
			device_context()->GSSetShaderResources(texture_id, 1, &ctx_texture->m_resource_view);
			device_context()->CSSetShaderResources(texture_id, 1, &ctx_texture->m_resource_view);
			device_context()->VSSetSamplers(sempler_id, 1, &ctx_texture->m_sempler);
			device_context()->PSSetSamplers(sempler_id, 1, &ctx_texture->m_sempler);
			device_context()->GSSetSamplers(sempler_id, 1, &ctx_texture->m_sempler);
			device_context()->CSSetSamplers(sempler_id, 1, &ctx_texture->m_sempler);
			//
			ctx_texture->m_texture_bind = true;
			ctx_texture->m_texture_id = texture_id;
			ctx_texture->m_sempler_id = sempler_id;
            //add this
            s_bind_context.m_textures[texture_id] = ctx_texture;
        }
	}

	void ContextDX11::unbind_texture(Texture* ctx_texture)
	{
        if (ctx_texture)
        {
			ID3D11SamplerState* sempler = nullptr;
			ID3D11ShaderResourceView* resource_view = nullptr;
			//
			device_context()->VSSetShaderResources(ctx_texture->m_texture_id, 1, &resource_view);
			device_context()->PSSetShaderResources(ctx_texture->m_texture_id, 1, &resource_view);
			device_context()->GSSetShaderResources(ctx_texture->m_texture_id, 1, &resource_view);
			device_context()->CSSetShaderResources(ctx_texture->m_texture_id, 1, &resource_view);
			device_context()->VSSetSamplers(ctx_texture->m_sempler_id, 1, &sempler);
			device_context()->PSSetSamplers(ctx_texture->m_sempler_id, 1, &sempler);
			device_context()->GSSetSamplers(ctx_texture->m_sempler_id, 1, &sempler);
			device_context()->CSSetSamplers(ctx_texture->m_sempler_id, 1, &sempler);
            //to null
            s_bind_context.m_textures[ctx_texture->m_texture_id] = nullptr;
			ctx_texture->m_texture_bind = false;
			ctx_texture->m_texture_id = 0;
			ctx_texture->m_sempler_id = 0;
        }
	}
        
    void ContextDX11::unbind_texture(int n)
    {
        unbind_texture(s_bind_context.m_textures[n]);
    }
        
	void ContextDX11::delete_texture(Texture*& ctx_texture)
    {
        //bind?
        if(ctx_texture->m_texture_bind)
        {
            unbind_texture(ctx_texture);
        }
        //safe delete
		delete ctx_texture;
		ctx_texture = nullptr;
	}

	/*
	Shader
	*/
	Shader::Shader()
	{
		for (auto& shader_binary : m_shader_binaries) shader_binary = nullptr;
	}

	Shader::~Shader()
	{
		//release shader bitecode
		for (auto& shader_binary : m_shader_binaries) if(shader_binary) shader_binary->Release();
		//release global context
		if(m_global_buffer_gpu) delete m_global_buffer_gpu;
		//release shaders
		if (m_vertex)   m_vertex->Release();
		if (m_pixel)    m_pixel->Release();
		if (m_geometry) m_geometry->Release();
		if (m_hull)     m_hull->Release();
		if (m_domain)   m_domain->Release();
		if (m_compute)  m_compute->Release();
	}

	void Shader::bind_global_buffer(ContextDX11* context)
	{
		if (m_global_buffer_gpu && m_global_buffer_should_be_bind)
		{
			context->update_steam_CB(
				  m_global_buffer_gpu
				, m_global_buffer_cpu.data()
				, m_global_buffer_cpu.size()
			);
		}		
		#define CBBIND(name_field, type, type_slot)\
			if (name_field)\
			{\
				if (m_global_buffer_gpu && m_global_buffer_info.m_slot[type_slot] >= 0)\
					context->device_context()->type##SetConstantBuffers(m_global_buffer_info.m_slot[type_slot]\
																	    , 1, &m_global_buffer_gpu->m_buffer);\
			}
		CBBIND(m_vertex, VS, ST_VERTEX_SHADER)
		CBBIND(m_pixel, PS, ST_FRAGMENT_SHADER)
		CBBIND(m_geometry, GS, ST_GEOMETRY_SHADER)
		CBBIND(m_hull, HS, ST_TASSELLATION_CONTROL_SHADER)
		CBBIND(m_domain, DS, ST_TASSELLATION_EVALUATION_SHADER)
		CBBIND(m_compute, CS, ST_COMPUTE_SHADER)
		#undef CBBIND
		m_global_buffer_should_be_bind = false;
	}
	void Shader::unbind_global_buffer(ContextDX11* context)
	{
		#define CBUNBIND(name_field, type, type_slot)\
			if (name_field)\
			{\
				if (m_global_buffer_gpu && m_global_buffer_info.m_slot[type_slot] >= 0)\
					context->device_context()->type##SetConstantBuffers(m_global_buffer_info.m_slot[type_slot]\
																	    , 0, nullptr);\
			}
		CBUNBIND(m_vertex, VS, ST_VERTEX_SHADER)
		CBUNBIND(m_pixel, PS, ST_FRAGMENT_SHADER)
		CBUNBIND(m_geometry, GS, ST_GEOMETRY_SHADER)
		CBUNBIND(m_hull, HS, ST_TASSELLATION_CONTROL_SHADER)
		CBUNBIND(m_domain, DS, ST_TASSELLATION_EVALUATION_SHADER)
		CBUNBIND(m_compute, CS, ST_COMPUTE_SHADER)
		#undef CBUNBIND
		m_global_buffer_should_be_bind = true;
	}

	static bool get_global_costant_buffer(
		  ID3DBlob* shader_blob
		, ShaderType type
		, Shader::GlobalBufferInfo& info
	)
	{
		ID3D11ShaderReflection *reflector = nullptr;
		D3DReflect(shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(), IID_ID3D11ShaderReflection, (void **)&reflector);
		D3D11_SHADER_DESC desc_shader;
		reflector->GetDesc(&desc_shader);
		for (UINT i = 0; i < desc_shader.ConstantBuffers; ++i)
		{
			auto global = reflector->GetConstantBufferByIndex(i);
			//shader desc
			D3D11_SHADER_BUFFER_DESC desc;
			global->GetDesc(&desc);
			//name of buffer
			std::string name(desc.Name);
			//info
			if ((name.find("$Global") != std::string::npos) || (name.find("_Global") != std::string::npos))
			{
				D3D11_SHADER_INPUT_BIND_DESC bind_desc;
				reflector->GetResourceBindingDescByName(desc.Name, &bind_desc);
				//save slot/name/size
				info.m_slot[type] = bind_desc.BindPoint;
				//test
				if (!info.m_found)
				{
					//ok
					info.m_found = true;
					info.m_name = std::move(name);
					info.m_size = desc.Size;
					//get all fields
					for (UINT v = 0; v < desc.Variables; ++v)
					{
						//get var
						auto* variable = global->GetVariableByIndex(v);
						//get desc
						D3D11_SHADER_VARIABLE_DESC desc_var;
						if (SUCCEEDED(variable->GetDesc(&desc_var)))
						{
							info.m_fields[desc_var.Name] = desc_var.StartOffset;
						}
					}
				}
			}
		}
		return true;
	}
	
	static bool get_global_costant_buffer_slot(
		  ID3DBlob* shader_blob
		, ShaderType type
		, Shader::GlobalBufferInfo& info
	)
	{
		ID3D11ShaderReflection *reflector = nullptr;
		D3DReflect(shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(), IID_ID3D11ShaderReflection, (void **)&reflector);
		D3D11_SHADER_DESC desc_shader;
		reflector->GetDesc(&desc_shader);
		for (UINT i = 0; i < desc_shader.ConstantBuffers; ++i)
		{
			auto global = reflector->GetConstantBufferByIndex(i);
			//shader desc
			D3D11_SHADER_BUFFER_DESC desc;
			global->GetDesc(&desc);
			//name of buffer
			std::string name(desc.Name);
			//info
			if ((name.find("$Global") != std::string::npos) || (name.find("_Global") != std::string::npos))
			{
				D3D11_SHADER_INPUT_BIND_DESC bind_desc;
				reflector->GetResourceBindingDescByName(desc.Name, &bind_desc);
				//save slot/name/size
				info.m_slot[type] = bind_desc.BindPoint;
			}
		}
		return true;
	}

	static void get_global_costant_buffer_uniforms(
		  ContextDX11* context
		, Shader* shader
		, unsigned char* global_cpu_buffer
		, Shader::GlobalBufferInfo& info
	)
	{
		for (auto pair : info.m_fields)
		{
			info.m_fields_uniforms.insert({ pair.first, UniformDX11(context, shader, global_cpu_buffer, pair.second) });
		}
	}
	
	static void get_global_texture_uniforms(
		  ContextDX11* context
		, ID3DBlob* shader_blob
		, Shader* shader
		, Shader::GlobalBufferInfo& info
	)
	{
		ID3D11ShaderReflection *reflector = nullptr;
		D3DReflect(shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(), IID_ID3D11ShaderReflection, (void **)&reflector);
		D3D11_SHADER_DESC desc_shader;
		reflector->GetDesc(&desc_shader);
		for (UINT i = 0; i < desc_shader.BoundResources; ++i)
		{
			D3D11_SHADER_INPUT_BIND_DESC bind_desc;
			if (SUCCEEDED(reflector->GetResourceBindingDesc(i, &bind_desc)))
			{
				//if is a texture // WIP, (D3D_SIT_TEXTURE) same location of texture                           
				if (bind_desc.Type == D3D_SIT_SAMPLER)
				{
					info.m_fields_uniforms[std::string(bind_desc.Name)] = UniformDX11(context, shader, nullptr, bind_desc.BindPoint);
				}
			}

		}
		
	}
	Shader* ContextDX11::create_shader(const std::vector< ShaderSourceInformation >& infos)
	{
		//alloc
		Shader* oshader = new Shader();
		//
		static const char* shader_version[ST_N_SHADER]
		{
			  "vs_5_0"
			, "ps_5_0"
			, "gs_5_0"
			, "hs_5_0"
			, "ds_5_0"
			, "cs_5_0"
		};
		//compile
		for (const ShaderSourceInformation& info : infos)
		{
			if (info.m_shader_source.size())
			{
				//start line
				std::string start_line = "\n#line " + std::to_string(info.m_line) + "\n";
				//source
				const std::string source
				{
					info.m_shader_header +
					Shader::hlsl_header_by_type[info.m_type] +
					Shader::hlsl_default_header +
					start_line +
					info.m_shader_source
				};
				//compile
				ID3DBlob* shader_blob = nullptr;
				ID3DBlob* error_blob = nullptr;
				if (D3DCompile2(
					  source.data()
					, source.size()
					, NULL
					, nullptr
					, nullptr
					, info.m_entry_point.c_str()
					, shader_version[info.m_type]
					,   D3DCOMPILE_OPTIMIZATION_LEVEL3  
				  /*  | D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY */ 
					, 0
					, 0
					, NULL
					, 0
					, &shader_blob
					, &error_blob
				) != S_OK)
				{
					//save error
					std::string err(static_cast<char*>(error_blob->GetBufferPointer()), error_blob->GetBufferSize());
					oshader->push_compiler_error({ info.m_type,  err.c_str() });
					//release
					error_blob->Release();
					if (shader_blob) shader_blob->Release();
					//to next shader
					continue;
				}
				//free error blob
				if(error_blob) error_blob->Release();
				//save blob
				oshader->m_shader_binaries[info.m_type] = shader_blob;
				//build buffer
				if (!oshader->m_global_buffer_gpu)
				{
					//get global info
					get_global_costant_buffer(shader_blob, info.m_type, oshader->m_global_buffer_info);
					//is != 0
					if(oshader->m_global_buffer_info.m_size)
					{
						oshader->m_global_buffer_gpu = create_stream_CB(nullptr, oshader->m_global_buffer_info.m_size);
						oshader->m_global_buffer_cpu.resize(oshader->m_global_buffer_info.m_size);
						get_global_costant_buffer_uniforms(this, oshader, oshader->m_global_buffer_cpu.data(), oshader->m_global_buffer_info);
					}
				}
				//get global slot
				get_global_costant_buffer_slot(shader_blob, info.m_type, oshader->m_global_buffer_info);
				//textures
				get_global_texture_uniforms(this, shader_blob, oshader, oshader->m_global_buffer_info);
				//type 
				switch (info.m_type)
				{
				case ST_VERTEX_SHADER:
					if (!dx_op_success(device()->CreateVertexShader(shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(), nullptr, &oshader->m_vertex)))
					{ oshader->push_compiler_error({ info.m_type,  "Error to compile bytecode" }); }
				break;
				case ST_FRAGMENT_SHADER:
					if (!dx_op_success(device()->CreatePixelShader(shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(), nullptr, &oshader->m_pixel)))
					{ oshader->push_compiler_error({ info.m_type,  "Error to compile bytecode" }); }
				break;
				case ST_GEOMETRY_SHADER:
					if (!dx_op_success(device()->CreateGeometryShader(shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(), nullptr, &oshader->m_geometry)))
					{ oshader->push_compiler_error({ info.m_type,  "Error to compile bytecode" }); }
				break;
				case ST_TASSELLATION_CONTROL_SHADER:
					if (!dx_op_success(device()->CreateHullShader(shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(), nullptr, &oshader->m_hull)))
					{ oshader->push_compiler_error({ info.m_type,  "Error to compile bytecode" }); }
				break;
				case ST_TASSELLATION_EVALUATION_SHADER:
					if (!dx_op_success(device()->CreateDomainShader(shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(), nullptr, &oshader->m_domain)))
					{ oshader->push_compiler_error({ info.m_type,  "Error to compile bytecode" }); }
				break;
				case ST_COMPUTE_SHADER:
					if (!dx_op_success(device()->CreateComputeShader(shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(), nullptr, &oshader->m_compute)))
					{ oshader->push_compiler_error({ info.m_type,  "Error to compile bytecode" }); }
				break;
				default: break;
				}
			}
		}
		//return shader
		return oshader;
	}

	bool ContextDX11::shader_compiled_with_errors(Shader* shader)
	{
		return shader->m_errors.size() != 0;
	}
	bool ContextDX11::shader_linked_with_error(Shader* shader)
	{
		return shader->m_liker_log.size() != 0;
	}

	std::vector< std::string > ContextDX11::get_shader_compiler_errors(Shader* shader)
	{
		std::vector< std::string > output;
		//state
		ShaderType last_type = ST_N_SHADER;
		//push
		for (auto& error_log : shader->m_errors)
		{
			if (last_type != error_log.m_type)
			{
				output.push_back(Shader::hlsl_shader_names[error_log.m_type]);
				last_type = error_log.m_type;
			}
			output.push_back(error_log.m_log);
		}
		return output;
	}
	std::string ContextDX11::get_shader_liker_error(Shader* shader)
	{
		return shader->m_liker_log;
	}

	void ContextDX11::bind_shader(Shader* shader)
	{
		if (s_bind_context.m_shader)
		{
			unbind_shader(s_bind_context.m_shader);
		}
		//save
		s_bind_context.m_shader = shader;
		//util define
		#define BIND(shader, name_field, type, type_slot)\
			if (shader->name_field)\
			{\
				device_context()->type##SetShader(shader->name_field, nullptr, 0);\
			}
		//bind all
		BIND(shader, m_vertex, VS, ST_VERTEX_SHADER)
		BIND(shader, m_pixel, PS, ST_FRAGMENT_SHADER)
		BIND(shader, m_geometry, GS, ST_GEOMETRY_SHADER)
		BIND(shader, m_hull, HS, ST_TASSELLATION_CONTROL_SHADER)
		BIND(shader, m_domain, DS, ST_TASSELLATION_EVALUATION_SHADER)
		BIND(shader, m_compute, CS, ST_COMPUTE_SHADER)
		//undef bind util
		#undef BIND
	}
	void ContextDX11::unbind_shader(Shader* shader)
	{
		if (shader)
		{
			square_assert(s_bind_context.m_shader == shader);
			//disable textures
			for (auto texture_ptr : s_bind_context.m_textures)
			{
				if (texture_ptr)
				{
					unbind_texture(texture_ptr);
				}
			}
		    //util define
			#define UNBIND(shader, name_field, type, type_slot)\
			if (shader->name_field)\
			{\
				device_context()->type##SetShader(nullptr, nullptr, 0);\
			}
			//disable program
			UNBIND(shader, m_vertex, VS, ST_VERTEX_SHADER)
			UNBIND(shader, m_pixel, PS, ST_FRAGMENT_SHADER)
			UNBIND(shader, m_geometry, GS, ST_GEOMETRY_SHADER)
			UNBIND(shader, m_hull, HS, ST_TASSELLATION_CONTROL_SHADER)
			UNBIND(shader, m_domain, DS, ST_TASSELLATION_EVALUATION_SHADER)
			UNBIND(shader, m_compute, CS, ST_COMPUTE_SHADER)
			//shader unbind
			shader->unbind_global_buffer(this);
			//undef bind util
			#undef UNBIND
			//to null
			s_bind_context.m_shader = nullptr;
		}
	}
    
    void ContextDX11::bind_uniform_CB(const ConstBuffer* buffer, Shader* shader, const std::string& uname)
    {
        //get
        auto* ucb = get_uniform_const_buffer(shader,uname);
        //bind
        if(ucb) ucb->bind(buffer);
    }
    
    void ContextDX11::unbind_uniform_CB(Shader* shader, const std::string& uname)
    {
        //get
        auto* ucb = get_uniform_const_buffer(shader,uname);
        //bind
        if(ucb) ucb->unbind();
    }
    
	void ContextDX11::delete_shader(Shader*& shader)
	{
		if (s_bind_context.m_shader == shader)
		{
			unbind_shader(shader);
		}
		delete shader;
		shader = nullptr;
	}

	Shader* ContextDX11::get_bind_shader() const
	{
		return s_bind_context.m_shader;
	}
    
    Uniform* ContextDX11::get_uniform(Shader* shader,const std::string& uname) const
    {
        auto uit = shader->m_global_buffer_info.m_fields_uniforms.find(uname);
        //if find
        if (uit != shader->m_global_buffer_info.m_fields_uniforms.end()) return &uit->second;
		//re try with _Global
		uit = shader->m_global_buffer_info.m_fields_uniforms.find("_Global_"+uname);
		//if find
		if (uit != shader->m_global_buffer_info.m_fields_uniforms.end()) return &uit->second;
        //else
        return nullptr;
    }
    
    UniformConstBuffer* ContextDX11::get_uniform_const_buffer(Square::Render::Shader* shader, const std::string& uname) const
    {
        auto uit = shader->m_uniform_const_buffer_map.find(uname);
        //if find
        if (uit != shader->m_uniform_const_buffer_map.end()) return uit->second.get();
		//valid?
		auto ucbuffer = MakeUnique<UniformConstBufferDX11>(allocator(),(ContextDX11*)this, shader, uname);
		//is valid?
		if (!ucbuffer->is_valid()) return nullptr;
        //add and return
		shader->m_uniform_const_buffer_map.insert({ 
			uname,
			std::move(ucbuffer)
		});
		return shader->m_uniform_const_buffer_map[uname].get();
    }

	/*
		FBO
	*/
	Target* ContextDX11::create_render_target(const std::vector< TargetField >& textures)
	{
		//create FBO
		auto target = new Target();
		//attach
		for (const TargetField& t_field : textures)
		{
			if (t_field.m_texture->m_type == DX_TEXTURE_2D)
			{
				if (t_field.m_type == RT_COLOR)
				{
					ID3D11RenderTargetView* rtarget_view;
					if (!dx_op_success(device()->CreateRenderTargetView(t_field.m_texture->id3d11_texture2D(), NULL, &rtarget_view)))
					{
						delete_render_target(target);
						return nullptr;
					}
					target->m_views.push_back(rtarget_view);
					target->m_view_textures.push_back(t_field.m_texture->id3d11_texture2D());
				}
				else if ((!target->m_depth) && (t_field.m_type == RT_DEPTH || t_field.m_type == RT_DEPTH_STENCIL))
				{
					ID3D11DepthStencilView* dstarget_view;
					D3D11_DEPTH_STENCIL_VIEW_DESC desc_dsv;
					ZeroMemory(&desc_dsv, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
					///////////////////////////////////////////////////////////////
					if(t_field.m_texture->m_is_cube)
					{
						desc_dsv.Format = t_field.m_texture->m_format_raw;
						desc_dsv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
						desc_dsv.Texture2DArray.ArraySize = 6;
					}
					else
					{
						desc_dsv.Format = t_field.m_texture->m_format_raw;
						desc_dsv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
					}
					///////////////////////////////////////////////////////////////
					if (!dx_op_success(device()->CreateDepthStencilView
					(
						  t_field.m_texture->id3d11_texture2D()
						, &desc_dsv
						, &dstarget_view
					)))
					{
						delete_render_target(target);
						return nullptr;
					}
					target->m_depth = dstarget_view;
					target->m_depth_texture = t_field.m_texture->id3d11_texture2D();
				}
			}
		}
		//return
		return target;
	}

	void ContextDX11::enable_render_target(Target* r_target)
	{
        if(r_target && s_bind_context.m_render_target != r_target)
        {
			device_context()->OMSetRenderTargets(r_target->m_views.size(), r_target->m_views.data(), r_target->m_depth);
            s_bind_context.m_render_target = r_target;
        }
	}

	void ContextDX11::disable_render_target(Target* r_target)
	{
        if(r_target)
        {
            square_assert(s_bind_context.m_render_target == r_target);
			device_context()->OMSetRenderTargets(0, nullptr, nullptr);
            s_bind_context.m_render_target = nullptr;
			//default target
			enable_render_target(m_view_target);
        }
	}

	void ContextDX11::delete_render_target(Target*& r_target)
    {
        //bind?
        if(s_bind_context.m_render_target == r_target)
        {
            disable_render_target(r_target);
        }
        //safe delete
		delete r_target;
		r_target = nullptr;
	}

	void ContextDX11::copy_target_to_target(
		const IVec4& from_area,
		Target* from,
		const IVec4& to_area,
		Target* to,
		TargetType	mask
	)
	{

		D3D11_BOX from_box;
		from_box.left = from_area.x;
		from_box.right = from_area.z;
		from_box.top = from_area.y;
		from_box.bottom = from_area.w;
		from_box.front = 0;
		from_box.back = 1;
		//copy
		if (mask != RT_DEPTH_STENCIL)
		{
			for (size_t i = 0; i < from->m_view_textures.size() && i < to->m_view_textures.size(); ++i)
			{
				device_context()->CopySubresourceRegion(
					  from->m_view_textures[i], 0
					, to_area.x, to_area.y, 0
					, to->m_view_textures[i], 0
					, &from_box
				);
			}
		}
		else
		{
			device_context()->CopySubresourceRegion(
				  from->m_depth_texture, 0
				, to_area.x, to_area.y, 0
				, to->m_depth_texture, 0
				, &from_box
			);
        }
	}

	bool ContextDX11::print_errors() const
	{
		std::stringstream output;
		if (m_errors.size())
		{
			for (auto& err : m_errors)
			{
				output << "DX11 error: " << err << std::endl;
			}
			m_errors.clear();
			//print
			std::cout << output.str() << std::endl;
			return true;
		}
		return false;
	}

	//Output file name and line
	bool ContextDX11::print_errors(const char* source_file_name, int line) const
	{
		std::stringstream output;
		if (m_errors.size())
		{
			output << "At file: " << source_file_name << " (" << line << ")" << ":" << std::endl;
			for(auto& err : m_errors) 
			{
					output << "DX11 error: " << err << std::endl;
			}
			m_errors.clear();
			//print
			std::cout << output.str() << std::endl;
			return true;
		}
		return false;
	}

//end render
};
}
