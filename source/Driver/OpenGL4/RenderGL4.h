//
//  Square
//
//  Created by Gabriele on 11/03/2018
//  Copyright � 2018 Gabriele. All rights reserved.
//
#include <string>
#include <vector>
#include <unordered_map>
#include "OpenGL4.h"
#include "Square/Config.h"
#include "Square/Driver/Render.h"


namespace Square
{
namespace Render
{
	//Class dec
    class  UniformGL4;
    class  UniformConstBufferGL4;
	class  ContextGL4;
	struct BindContext;

	//BUFFER
	class ContextBuffer
	{
	public:

		GLuint m_id_buffer;
        size_t m_size;

		ContextBuffer(GLuint id = 0) :m_id_buffer(id), m_size(0) {}

		inline operator GLuint() const
		{
			return m_id_buffer;
		}

		inline operator GLuint*()
		{
			return &m_id_buffer;
		}

		void gen_buffer()
		{
			glGenBuffers(1, &m_id_buffer);
		}
        
        void set_size(size_t size)
        {
            m_size = size;
        }
        
        size_t get_size() const
        {
            return m_size;
        }
	};

	class ConstBuffer : public ContextBuffer
	{
	public:
		ConstBuffer(GLuint id = 0) :ContextBuffer(id) {}
	};

	class VertexBuffer : public ContextBuffer
	{
	public:
		VertexBuffer(GLuint id = 0) :ContextBuffer(id) {}
	};

	class IndexBuffer : public ContextBuffer
	{
	public:
		IndexBuffer(GLuint id = 0) :ContextBuffer(id) {}
	};

	//INPUT LAYOUT
	class InputLayout
	{
	public:
		//locations
		using LocationList = std::vector< GLint >;
		//location
		struct GLLayout
		{
			LocationList  m_locations;
			AttributeList m_attributes;
			size_t size() const
			{
				return m_locations.size();
			}
		};
		//map
		using GLLayoutMap = std::unordered_map< Shader*, GLLayout >;
		//OpenGL info
		GLLayoutMap m_map_layout;
		//info given from appliction
		AttributeList m_list;
		//utils
		GLLayout* get(Shader* shader);

	private:
		//build
		GLLayout build(Shader* shader);
	};

	//TEXTURE
	class Texture
	{
	public:
		GLenum       m_type_texture{ GL_TEXTURE_2D };
		int          m_last_bind{ -1 };
		unsigned int m_tbo{ 0 };

		Texture() {}

		virtual ~Texture()
		{
			if (m_tbo) glDeleteTextures(1, &m_tbo);
		}

		inline void create_TBO()
		{
			glGenTextures(1, &m_tbo);
		}

		inline void enable_TBO(int n)
		{
			m_last_bind = n;
			glActiveTexture((GLenum)(GL_TEXTURE0 + m_last_bind));
			glBindTexture(m_type_texture, m_tbo);
		}

		inline void disable_TBO()
		{
			if (m_last_bind >= 0)
			{
				glActiveTexture((GLenum)(GL_TEXTURE0 + m_last_bind));
				glBindTexture(m_type_texture, (GLuint)0);
				m_last_bind = -1;
			}
		}

	};

	//FRAMES TARGET
	class Target
	{
	public:

		GLuint m_fbo{ 0 };

		virtual ~Target()
		{
			if (m_fbo) glDeleteFramebuffers(1, &m_fbo);
		}
		//fbo
		void gen_FBO()
		{
			//vbo
			glGenFramebuffers(1, &m_fbo);
		}

		void enable_FBO()
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		}

		void disable_FBO()
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	};
   
	//UNIFORM
    class UniformGL4 : public Uniform
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
        
        UniformGL4(ContextGL4* context, Shader* shader, GLint id);
        
        UniformGL4();
        
        virtual ~UniformGL4();
        
    protected:
        ContextGL4* m_context;
        Shader*     m_shader;
        GLint       m_id;
        
    };
    
	//UNIFORM Global UBO
	class UniformGLGUBO : public Uniform
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

		UniformGLGUBO(ContextGL4* context, Shader* shader, unsigned char* buffer, size_t offset);

		UniformGLGUBO();

		virtual ~UniformGLGUBO();

	protected:
		ContextGL4 *   m_context{ nullptr };
		Shader*        m_shader{ nullptr };
		size_t         m_offset{ 0 };
		unsigned char* m_buffer{ nullptr };
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
    class UniformConstBufferGL4 : public UniformConstBuffer
    {
    public:
        //create buffer ref
        UniformConstBufferGL4(ContextGL4* context, Shader* shader, GLint id, GLuint  bind);
        UniformConstBufferGL4();
        //bind
        virtual void bind(const ConstBuffer*);
        virtual void unbind();
        //buffer info
        virtual bool is_valid();
        virtual Shader* get_shader();
        virtual ~UniformConstBufferGL4();
        
    protected:
		friend class Shader;
        const ConstBuffer* m_const_buffer;
        ContextGL4*        m_context;
        Shader*            m_shader;
		GLint              m_id;
		GLuint             m_bind;
    };
	
	//Shader
	class Shader
	{
	public:
		/////////////////////////////////////////////////////////////////////////
        using UniformMap             = std::unordered_map< std::string, Unique<Uniform> >;
        using UniformPair            = std::pair< std::string, Unique<Uniform> >;
        using UniformConstBufferMap  = std::unordered_map< std::string, UniformConstBufferGL4 >;
        using UniformConstBufferPair = std::pair< std::string, UniformConstBufferGL4 >;
        //shader compile errors
        struct ShaderCompileError
        {
            ShaderType m_type;
            std::string m_log;
        };
		/////////////////////////////////////////////////////////////////////////
		static const char* glsl_default_header;
		static const char* glsl_header_by_type[ST_N_SHADER];
		static const char* glsl_shader_names[ST_N_SHADER];
		static GLenum      glsl_type_from_square_type[ST_N_SHADER];
		/////////////////////////////////////////////////////////////////////////
		Shader(ContextGL4& context) : m_context(context) { }
		
		//delete
		~Shader();

		//bind / unbind
		void bind();
		void unbind();

		//new bind index
		unsigned int get_new_constat_buffer_bind_index();

		//update global buffer
		void global_buffer_bind();

		//get uniform
		Uniform* uniform(const std::string& name);

		//get uniform const buffer
		UniformConstBuffer* uniform_const_buffer(const std::string& name);

        //add error log
		void push_compiler_error(const ShaderCompileError& error_log);
        
		void push_liker_error(const std::string& error_log);

		//shader id
		unsigned int shader_id() const { return m_shader_id; }
        
    protected:
        //friends
        friend class ContextGL4;
		friend class UniformGL4;
		friend class UniformGLGUBO;
        friend class UniformConstBufferGL4;

		//Context
		ContextGL4& m_context;

		//global uniforms
		bool						 m_global_buffer_update{ true };
		UniformConstBufferGL4*		 m_global_buffer_ref{ nullptr };
		ConstBuffer*				 m_global_buffer_gpu{ nullptr };
		std::vector< unsigned char > m_global_buffer_cpu;
		void build_global_UBO();
		void destroy_global_ubo();

		//uniforms
		mutable UniformMap m_uniform_map;
        mutable UniformConstBufferMap m_uniform_const_buffer_map;
		mutable long m_uniform_ntexture{ -1 }; //n texture bind
        
        //help
		Uniform& add_uniform(const std::string& name, Unique<Uniform>&& u) const;
		UniformConstBufferGL4& add_uniform_const_buffer(const std::string& name, const UniformConstBufferGL4& ucb) const;
        
		//context
		unsigned int m_bind_cb_index{ 0 };
		unsigned int m_shader_id{ 0 };      // shader program
		unsigned int m_shaders[ST_N_SHADER];// shaders

		//shaders compiler errors
		std::vector < ShaderCompileError > m_errors;

		//linking error
		std::string m_liker_log;
        
	};
    
	//Bind context
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
	class ContextGL4 : public Context
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
		virtual void draw_elements(DrawType type, unsigned int start, unsigned int n) override;

		//IL=Input Layaut
		virtual InputLayout* create_IL(const AttributeList& atl) override;
		virtual void delete_IL(InputLayout*&) override;

		virtual void bind_IL(InputLayout*) override;
		virtual void unbind_IL(InputLayout* il) override;

		virtual size_t size_IL(const InputLayout* layout) override;
		virtual bool   has_a_position_IL(const InputLayout* layout) override;
		virtual size_t position_offset_IL(const InputLayout* layout) override;

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
		virtual  std::vector< unsigned char > get_texture(Texture*, int face = 0, int level = 0) override;
		virtual void bind_texture(Texture*, int texture_id, int sempler_id) override;
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
		GLuint             s_vao_attributes;
		RenderDriverInfo   s_render_driver_info;
	};
}
}
