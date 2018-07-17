//
//  Square
//
//  Created by Gabriele on 11/03/2018
//  Copyright � 2018 Gabriele. All rights reserved.
//
#include <list>
#include <string>
#include <vector>
#include <unordered_map>
#include "DirectX11.h"
#include "Square/Config.h"
#include "Square/Driver/Render.h"


namespace Square
{
namespace Video
{
	struct DeviceResources;
}
namespace Render
{
	//Class dec
    class  UniformDX11;
    class  UniformConstBufferDX11;
	class  ContextDX11;
	class  Shader;
	struct BindContext;

	//BUFFER
	class ContextBuffer
	{
	public:

		ID3D11Buffer* m_buffer;
		size_t m_size;

		ContextBuffer(ID3D11Buffer* buffer = nullptr, size_t size = 0) :m_buffer(buffer), m_size(size) {}

		inline operator ID3D11Buffer* const() const { return m_buffer; }
		        
        void set_size(size_t size)  {  m_size = size; }
        
        size_t get_size() const {  return m_size; }

		virtual ~ContextBuffer() { if(m_buffer) m_buffer->Release(); }
	};

	class ConstBuffer : public ContextBuffer
	{
	public:
		ConstBuffer(ID3D11Buffer* buffer = nullptr, size_t size = 0) :ContextBuffer(buffer, size) {}
	};

	class VertexBuffer : public ContextBuffer
	{
	public:
		UINT m_offset;
		UINT m_stride;
		UINT m_n;

		VertexBuffer(ID3D11Buffer* buffer = nullptr, UINT stride = 0, UINT n = 0)
		: ContextBuffer(buffer, stride*n) 
		, m_offset(0)
		, m_stride(stride)
		, m_n(n)
		{
		}

		void set_stride(UINT size) { m_stride = size; set_size(m_stride*get_n()); }

		UINT get_stride() const { return m_stride; }

		void set_n(UINT size) { m_n = size; set_size(get_stride()*m_n); }

		UINT get_n() const { return m_n; }
	};

	class IndexBuffer : public ContextBuffer
	{
	public:
		IndexBuffer(ID3D11Buffer* buffer = nullptr, size_t size = 0) :ContextBuffer(buffer, size) {}
	};

	//INPUT LAYOUT
	class InputLayout
	{
	protected:

		ID3D11InputLayout* build(ContextDX11*, ID3DBlob* shader);

	public:
		//for shader
		AttributeList							m_attributes;
		std::vector< D3D11_INPUT_ELEMENT_DESC > m_description;
		ID3D11InputLayout*						m_layout{ nullptr }; //only for vertex shader
		//build
		InputLayout(ContextDX11* context11, Shader* shader, const AttributeList& attributes, const std::vector< D3D11_INPUT_ELEMENT_DESC >& description);
		operator ID3D11InputLayout*() { return m_layout; }
		virtual ~InputLayout();
	};

	//TEXTURE
	enum TextureTypeDX11
	{
		DX_TEXTURE_NONE,
		DX_TEXTURE_1D,
		DX_TEXTURE_2D,
		DX_TEXTURE_3D,
	};

	class Texture
	{
	public:
		TextureTypeDX11	          m_type{ DX_TEXTURE_NONE };
		ID3D11SamplerState*       m_sempler{ nullptr };
		ID3D11ShaderResourceView* m_resource_view{ nullptr };
		UINT					  m_width{ 0 };
		UINT					  m_height{ 0 };
		bool					  m_is_cube{ false };

		bool m_texture_bind{ false };
		UINT m_texture_id{ 0 };
		UINT m_sempler_id{ 0 };

		virtual operator ID3D11Texture1D*() { return nullptr; }
		virtual operator ID3D11Texture2D*() { return nullptr; }
		virtual operator ID3D11Texture3D*() { return nullptr; }

		virtual ~Texture()
		{ 
			if (m_sempler) m_sempler->Release(); 
			if (m_resource_view) m_resource_view->Release();
		}
	};

	class Texture1D : public Texture
	{
	public:
		Texture1D() { m_type = DX_TEXTURE_1D; }
		ID3D11Texture1D* m_texture1D{ nullptr };
		virtual operator ID3D11Texture1D*() { return m_texture1D; }
		virtual ~Texture1D(){ if (m_texture1D) m_texture1D->Release(); }
	};

	class Texture2D : public Texture
	{
	public:
		Texture2D() { m_type = DX_TEXTURE_2D; }
		ID3D11Texture2D* m_texture2D{ nullptr };
		virtual operator ID3D11Texture2D*() { return m_texture2D; }
		virtual ~Texture2D() { if (m_texture2D) m_texture2D->Release(); }
	};

	class Texture3D : public Texture
	{
	public:
		Texture3D() { m_type = DX_TEXTURE_3D; }
		ID3D11Texture3D * m_texture3D{ nullptr };
		virtual operator ID3D11Texture3D*() { return m_texture3D; }
		virtual ~Texture3D() { if (m_texture3D) m_texture3D->Release(); }
	};
	
	//frames target
	class Target
	{
	public:
		//init
		Target(bool dealloc = true)
		{
			m_dealloc = dealloc;
		}
		//dealloc flag
		bool m_dealloc{ true };
		//views
		std::vector<ID3D11RenderTargetView*> m_views;
		ID3D11DepthStencilView* m_depth{ nullptr };
		//copy buffer, wake ref
		std::vector<ID3D11Texture2D*> m_view_textures;
		ID3D11Texture2D*			  m_depth_texture{ nullptr };
		//decallo
		virtual ~Target()
		{ 
			if (m_dealloc)
			{
				for (auto view : m_views) view->Release();
				if (m_depth) m_depth->Release();
			}
		}
	};

    //uniform
    class UniformDX11 : public Uniform
    {
    public:
        virtual void set(Texture* in_texture) override;
        virtual void set(int i) override;
        virtual void set(float f) override;
        virtual void set(double d) override;
        
        virtual void set(const IVec2& v2) override;
        virtual void set(const IVec3& v3) override;
        virtual void set(const IVec4& v4) override;
        
        virtual void set(const Vec2& v2) override;
        virtual void set(const Vec3& v3) override;
        virtual void set(const Vec4& v4) override;
        virtual void set(const Mat3& m3) override;
        virtual void set(const Mat4& m4) override;
        
        virtual void set(const DVec2& v2) override;
        virtual void set(const DVec3& v3) override;
        virtual void set(const DVec4& v4) override;
        virtual void set(const DMat3& m3) override;
        virtual void set(const DMat4& m4) override;
        
        virtual void set(Texture* t, size_t n) override;
        virtual void set(const int* i, size_t n) override;
        virtual void set(const float* f, size_t n) override;
        virtual void set(const double* d, size_t n) override;
        
        virtual void set(const IVec2* v2, size_t n) override;
        virtual void set(const IVec3* v3, size_t n) override;
        virtual void set(const IVec4* v4, size_t n) override;
        
        virtual void set(const Vec2* v2, size_t n) override;
        virtual void set(const Vec3* v3, size_t n) override;
        virtual void set(const Vec4* v4, size_t n) override;
        virtual void set(const Mat3* m3, size_t n) override;
        virtual void set(const Mat4* m4, size_t n) override;
        
        virtual void set(const DVec2* v2, size_t n) override;
        virtual void set(const DVec3* v3, size_t n) override;
        virtual void set(const DVec4* v4, size_t n) override;
        virtual void set(const DMat3* m3, size_t n) override;
        virtual void set(const DMat4* m4, size_t n) override;
        
        virtual void set(const std::vector < Texture* >& t) override;
        virtual void set(const std::vector < int >& i)      override;
        virtual void set(const std::vector < float >& f)    override;
        virtual void set(const std::vector < double >& d)   override;
        
        virtual void set(const std::vector < IVec2 >& v2) override;
        virtual void set(const std::vector < IVec3 >& v3) override;
        virtual void set(const std::vector < IVec4 >& v4) override;
        
        virtual void set(const std::vector < Vec2 >& v2) override;
        virtual void set(const std::vector < Vec3 >& v3) override;
        virtual void set(const std::vector < Vec4 >& v4) override;
        virtual void set(const std::vector < Mat3 >& m3) override;
        virtual void set(const std::vector < Mat4 >& m4) override;
        
        virtual void set(const std::vector < DVec2 >& v2) override;
        virtual void set(const std::vector < DVec3 >& v3) override;
        virtual void set(const std::vector < DVec4 >& v4) override;
        virtual void set(const std::vector < DMat3 >& m3) override;
        virtual void set(const std::vector < DMat4 >& m4) override;
        
        virtual bool is_valid() override;
        
        virtual Shader* get_shader() override;
        
        UniformDX11(ContextDX11* context, Shader* shader, unsigned char* buffer, size_t offset);
        
        UniformDX11();
        
        virtual ~UniformDX11();
        
    protected:
        ContextDX11*   m_context{ nullptr };
        Shader*        m_shader { nullptr };
		size_t         m_offset { 0 };
		unsigned char* m_buffer { nullptr };
		//write
		template < typename T > void primitive_write(const T& value)
		{
			std::memcpy(m_buffer + m_offset, &value, sizeof(T));
		}
		template < typename T > void array_write(const T* value, size_t n)
		{
			std::memcpy(m_buffer + m_offset, value, sizeof(T)*n);
		}
    };
    
    //buffer
    class UniformConstBufferDX11 : public UniformConstBuffer
    {
    public:
        //create buffer ref
        UniformConstBufferDX11(ContextDX11* context, Shader* shader, const std::string& cname);
        UniformConstBufferDX11();
        //bind
        virtual void bind(const ConstBuffer*);
        virtual void unbind();
        //buffer info
        virtual bool is_valid();
        virtual Shader* get_shader();
        virtual ~UniformConstBufferDX11();
        
    protected:
		bool	     m_found_the_cbuffer{ false };
		ContextDX11* m_context{ nullptr };
		Shader*      m_shader{ nullptr };
		INT          m_slots[ST_N_SHADER];
    };
	
	//shader
	class Shader
	{
	public:
		/////////////////////////////////////////////////////////////////////////       
		using UniformConstBufferSlots = std::unordered_map< std::string, UINT >;
        using UniformMap              = std::unordered_map< std::string, UniformDX11 >;
        using UniformPair             = std::pair< std::string, UniformDX11 >;
        using UniformConstBufferMap   = std::unordered_map< std::string, Unique< UniformConstBufferDX11 > >;
        //shader compile errors
        struct ShaderCompileError
        {
            ShaderType m_type;
            std::string m_log;
        };
		struct GlobalBufferInfo
		{
			std::string m_name;
			bool		m_found{ false };
			INT         m_slot[ST_N_SHADER];
			size_t      m_size{ 0 };
			std::map<std::string, size_t> m_fields;
			std::map<std::string, UniformDX11> m_fields_uniforms;

			GlobalBufferInfo()
			{
				for (auto& slot : m_slot) slot = -1;
			}
		};
		/////////////////////////////////////////////////////////////////////////
		static const char* hlsl_default_header;
		static const char* hlsl_header_by_type[ST_N_SHADER];
		static const char* hlsl_shader_names[ST_N_SHADER];
		/////////////////////////////////////////////////////////////////////////
		//init
		Shader();
		//delete
		~Shader();

        //add error log
        void push_compiler_error(const ShaderCompileError& error_log)
        {
            m_errors.push_back(std::move(error_log));
        }
        
        void push_liker_error(const std::string& error_log)
        {
            m_liker_log += error_log;
            m_liker_log += "\n";
        }

		ID3DBlob* binary(ShaderType type)
		{
			return m_shader_binaries[type];
		}

		void global_buffer_bind()
		{
			m_global_buffer_should_be_bind = true;
		}

		void bind_global_buffer(ContextDX11* context);
		void unbind_global_buffer(ContextDX11* context);

    protected:
        //friends
        friend class ContextDX11;
        friend class UniformDX11;
        friend class UniformConstBufferDX11;

		//shader binary code
		ID3DBlob* m_shader_binaries[ST_N_SHADER];

		//uniforms
		mutable UniformMap m_uniform_map;
        mutable UniformConstBufferMap m_uniform_const_buffer_map;
		mutable UniformConstBufferSlots m_uniform_const_buffer_slot_map;
		mutable long m_uniform_ntexture{ -1 }; //n texture bind

		//global uniforms
		bool						 m_global_buffer_should_be_bind{ true };
		GlobalBufferInfo			 m_global_buffer_info;
		ConstBuffer*				 m_global_buffer_gpu{ nullptr };
		std::vector< unsigned char > m_global_buffer_cpu;
        
        //help
        UniformDX11& add_uniform(const std::string& name, const UniformDX11& u) const
        {
            auto& uref = (m_uniform_map[name] = u);
            return uref;
        }
        
		//context
		ID3D11VertexShader*   m_vertex{ nullptr };
		ID3D11PixelShader*    m_pixel{ nullptr };
		ID3D11GeometryShader* m_geometry{ nullptr };
		ID3D11HullShader*     m_hull{ nullptr };
		ID3D11DomainShader*   m_domain{ nullptr };
		ID3D11ComputeShader*  m_compute{ nullptr };;
		
		//shaders compiler errors
		std::vector < ShaderCompileError > m_errors;

		//linking error
		std::string m_liker_log;
        
	};
    
	//BIND CONTEXT
	struct BindContext
	{
		Texture*       m_textures[32]{ nullptr };
		Texture*	   m_texture_buffer{ nullptr };
		ConstBuffer*   m_const_buffer{ nullptr };
		VertexBuffer*  m_vertex_buffer{ nullptr };
		IndexBuffer*   m_index_buffer{ nullptr };
		InputLayout*   m_input_layout{ nullptr };
		Target*		   m_render_target{ nullptr };
		Shader*		   m_shader{ nullptr };
	};
	
	//CONTEXT
	class ContextDX11 : public Context
	{
	public:

		virtual RenderDriver get_render_driver() override;
		virtual RenderDriverInfo get_render_driver_info() override;
		virtual void print_info() override;

		virtual bool init(Video::DeviceResources* resource) override;
		virtual void close() override;

		virtual const ClearColorState& get_clear_color_state() override;
		virtual void set_clear_color_state(const ClearColorState& cf) override;
		virtual void clear(int type = CLEAR_COLOR_DEPTH) override;

		virtual const DepthBufferState& get_depth_buffer_state() override;
		virtual void set_depth_buffer_state(const DepthBufferState& cf) override;

		virtual const CullfaceState& get_cullface_state() override;
		virtual void set_cullface_state(const CullfaceState& cf) override;

		virtual const ViewportState& get_viewport_state() override;
		virtual void set_viewport_state(const ViewportState& vs) override;

		virtual const BlendState& get_blend_state() override;
		virtual void set_blend_state(const BlendState& bs) override;

		virtual const State& get_render_state() override;
		virtual void set_render_state(const State& rs) override;

		//BO
		virtual ConstBuffer* create_stream_CB(const unsigned char* data, size_t size) override;
		virtual VertexBuffer* create_stream_VBO(const unsigned char* vbo, size_t stride, size_t n) override;
		virtual IndexBuffer* create_stream_IBO(const unsigned int* ibo, size_t size) override;

		virtual ConstBuffer* create_CB(const unsigned char* data, size_t size) override;
		virtual VertexBuffer* create_VBO(const unsigned char* vbo, size_t stride, size_t n) override;
		virtual IndexBuffer* create_IBO(const unsigned int* ibo, size_t size) override;

		virtual Variant get_native_CB(const ConstBuffer*) const override;
		virtual Variant get_native_VBO(const VertexBuffer*) const override;
		virtual Variant get_native_IBO(const IndexBuffer*) const override;

		virtual size_t get_size_CB(const ConstBuffer*) const override;
		virtual size_t get_size_VBO(const VertexBuffer*) const override;
		virtual size_t get_size_IBO(const IndexBuffer*) const override;

		virtual void update_steam_CB(ConstBuffer* cb, const unsigned char* vb, size_t n) override;
		virtual void update_steam_VBO(VertexBuffer* vbo, const unsigned char* vb, size_t n) override;
		virtual void update_steam_IBO(IndexBuffer* vbo, const unsigned int* ib, size_t n) override;

		virtual void bind_CB(ConstBuffer*) override;
		virtual void bind_VBO(VertexBuffer*) override;
		virtual void bind_IBO(IndexBuffer*) override;

		virtual void unbind_CB(ConstBuffer*) override;
		virtual void unbind_VBO(VertexBuffer*) override;
		virtual void unbind_IBO(IndexBuffer*) override;

		virtual unsigned char* map_CB(ConstBuffer*, size_t start, size_t n, MappingType type) override;
		virtual void unmap_CB(ConstBuffer*) override;

		virtual unsigned char* map_VBO(VertexBuffer*, size_t start, size_t n, MappingType type) override;
		virtual void unmap_VBO(VertexBuffer*) override;

		virtual unsigned int*  map_IBO(IndexBuffer*, size_t start, size_t n, MappingType type) override;
		virtual void unmap_IBO(IndexBuffer*) override;

		virtual unsigned char* map_TBO(Texture*, MappingType type) override;
		virtual void unmap_TBO(Texture*) override;

		virtual void delete_CB(ConstBuffer*&) override;
		virtual void delete_VBO(VertexBuffer*&) override;
		virtual void delete_IBO(IndexBuffer*&) override;
		//draw
		virtual void draw_arrays(DrawType type, unsigned int n) override;
		virtual void draw_arrays(DrawType type, unsigned int start, unsigned int size) override;
		virtual void draw_elements(DrawType type, unsigned int n) override;

		//IL=Input Layaut
		virtual InputLayout* create_IL(Shader* shader, const AttributeList& atl) override;
		virtual size_t size_IL(const InputLayout* layout) override;
		virtual bool   has_a_position_IL(const InputLayout* layout) override;
		virtual size_t position_offset_IL(const InputLayout* layout) override;
		virtual void delete_IL(InputLayout*&) override;
		virtual void bind_IL(InputLayout*) override;
		virtual void unbind_IL(InputLayout* il) override;

		//depth
		virtual float get_depth(const Vec2& pixel) const override;

		//color
		virtual Vec4 get_color(const Vec2& pixel) const override;

		//texture
		virtual Texture* create_texture
		(
			const TextureRawDataInformation& data,
			const TextureGpuDataInformation& info
		) override;
		virtual Texture* create_cube_texture
		(
			const TextureRawDataInformation  data[6],
			const TextureGpuDataInformation& info
		) override;
		virtual  std::vector< unsigned char > get_texture(Texture*, int level = 0) override;
		virtual  std::vector< unsigned char > get_texture(Texture*, int cube = 0, int level = 0) override;
		virtual void bind_texture(Texture*, int texture_id, int sample_id) override;
		virtual void unbind_texture(Texture*) override;
		virtual void unbind_texture(int n) override;
		virtual void delete_texture(Texture*&) override;

		//shader
		virtual Shader* create_shader(const std::vector< ShaderSourceInformation >& infos) override;

		virtual bool shader_compiled_with_errors(Shader* shader) override;
		virtual bool shader_linked_with_error(Shader* shader) override;
		virtual std::vector< std::string > get_shader_compiler_errors(Shader* shader) override;
		virtual std::string get_shader_liker_error(Shader* shader) override;

		virtual void bind_shader(Shader* shader) override;
		virtual void unbind_shader(Shader* shader) override;

		virtual void bind_uniform_CB(const ConstBuffer*, Shader*, const std::string& uname) override;
		virtual void unbind_uniform_CB(Shader*, const std::string& uname) override;

		virtual void delete_shader(Shader*&) override;

		virtual Shader*  get_bind_shader() const override;
		virtual Uniform* get_uniform(Shader*, const std::string& uname) const override;
		virtual UniformConstBuffer* get_uniform_const_buffer(Shader*, const std::string& uname) const override;

		//target
		virtual Target* create_render_target(const std::vector< TargetField >& textures) override;
		virtual void enable_render_target(Target*) override;
		virtual void disable_render_target(Target*) override;
		virtual void delete_render_target(Target*&) override;
		//copy target
		virtual void copy_target_to_target(
			const IVec4& from_area,
			Target* from,
			const IVec4& to_area,
			Target* to,
			TargetType	mask
		) override;

		//debug
		virtual bool print_errors() const override;
		//Output file name and line
		virtual bool print_errors(const char* source_file_name, int line) const override;

		//context
		BindContext        s_bind_context;
		State			   s_render_state;
		RenderDriverInfo   s_render_driver_info;

		///////////////////////////////////////////////////////////////////////////////////
		//help
		ID3D11Device* device() const;
		IDXGISwapChain* swap() const;
		ID3D11DeviceContext* device_context() const;
		//device
		mutable ID3D11Device* m_device{ nullptr };
		//swap
		mutable IDXGISwapChain* m_swap{ nullptr };
		//Context
		mutable ID3D11DeviceContext* m_device_context{ nullptr };
		//Ref to device resouces 
		mutable Video::DeviceResources* m_device_resouces{ nullptr };
		//Init device, swap and context
		bool get_devices(Video::DeviceResources* resource);
		///////////////////////////////////////////////////////////////////////////////////
		//Depth buffer
		Target*	m_view_target;
		bool get_view_target(Video::DeviceResources* resource);
		///////////////////////////////////////////////////////////////////////////////////
		//query
		Texture2D*	m_query_buffer[2]{ nullptr, nullptr };
		bool build_query_buffer();
		///////////////////////////////////////////////////////////////////////////////////
		//render states
		ID3D11RasterizerState*	m_render_state_cullface_back;
		ID3D11RasterizerState*	m_render_state_cullface_front;
		ID3D11RasterizerState*	m_render_state_cullface_back_and_front;
		ID3D11RasterizerState*	m_render_state_cullface_disable;
		bool build_cullface_states();
		ID3D11RasterizerState* cullface_state(CullfaceState);
		///////////////////////////////////////////////////////////////////////////////////
		std::map<DepthBufferState, ID3D11DepthStencilState*> m_depth_states;
		ID3D11DepthStencilState* depth_state(DepthBufferState);
		///////////////////////////////////////////////////////////////////////////////////
		std::map<BlendState, ID3D11BlendState*>	m_blend_states;
		ID3D11BlendState* blend_state(BlendState);
		///////////////////////////////////////////////////////////////////////////////////
		bool dx_op_success(HRESULT h) const;
		mutable std::list<std::string> m_errors;
		///////////////////////////////////////////////////////////////////////////////////
	};
}
}
