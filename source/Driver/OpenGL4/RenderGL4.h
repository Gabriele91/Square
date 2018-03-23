//
//  Square
//
//  Created by Gabriele on 11/03/2018
//  Copyright © 2018 Gabriele. All rights reserved.
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
	class  ContextGL4;
	struct BindContext;

	//BUFFER
	class ContextBuffer
	{
	public:

		GLuint m_id_buffer;

		ContextBuffer(GLuint id = 0) :m_id_buffer(id) {}

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
		AttributeList m_list;
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

	//SHADER
	class Shader
	{
	public:
		/////////////////////////////////////////////////////////////////////////
		using UniformMap = std::unordered_map< std::string, UniformGL4 >;
		/////////////////////////////////////////////////////////////////////////
		static const char* glsl_default_header;
		static const char* glsl_header_by_type[ST_N_SHADER];
		static const char* glsl_shader_names[ST_N_SHADER];
		static GLenum      glsl_type_from_square_type[ST_N_SHADER];
		/////////////////////////////////////////////////////////////////////////
		//delete
		~Shader()
		{
			//detach and delete all shader
			if (m_shader_id)
			{
				for (unsigned int& shader : m_shaders)
				{
					if (shader)
					{
						//detach
						glDetachShader(m_shader_id, shader);
						//delete
						glDeleteShader(shader);
						//to null
						shader = 0;
					}
				}
				//delete shader program
				glDeleteProgram(m_shader_id);
				//to null
				m_shader_id = 0;
			}
		}

		//uniforms
		UniformMap m_uniform_map;
		long m_uniform_ntexture{ -1 }; //n texture bind

		//context
		unsigned int m_shader_id{ 0 };      //< shader program
		unsigned int m_shaders[ST_N_SHADER];//< shaders

		//shader compile errors
		struct ShaderCompileError
		{
			ShaderType m_type;
			std::string m_log;
		};

		//shaders compiler errors
		std::vector < ShaderCompileError > m_errors;

		//linking error
		std::string m_liker_log;

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

	};
	
	//UNIFORM
	class UniformGL4 : public Uniform
	{
	public:
		virtual void set_value(Texture* in_texture);
		virtual void set_value(int i);
		virtual void set_value(float f);
		virtual void set_value(const Vec2& v2);
		virtual void set_value(const Vec3& v3);
		virtual void set_value(const Vec4& v4);
		virtual void set_value(const Mat3& m3);
		virtual void set_value(const Mat4& m4);

		virtual void set_value(const int* i, size_t n);
		virtual void set_value(const float* f, size_t n);
		virtual void set_value(const Vec2* v2, size_t n);
		virtual void set_value(const Vec3* v3, size_t n);
		virtual void set_value(const Vec4* v4, size_t n);
		virtual void set_value(const Mat3* m3, size_t n);
		virtual void set_value(const Mat4* m4, size_t n);

		virtual void set_value(const std::vector < int >& i);
		virtual void set_value(const std::vector < float >& f);
		virtual void set_value(const std::vector < Vec2 >& v2);
		virtual void set_value(const std::vector < Vec3 >& v3);
		virtual void set_value(const std::vector < Vec4 >& v4);
		virtual void set_value(const std::vector < Mat3 >& m3);
		virtual void set_value(const std::vector < Mat4 >& m4);

		virtual void set_value(const ConstBuffer*);

		virtual bool is_valid();

		virtual Shader* get_shader();


		UniformGL4(ContextGL4* context, Shader* shader, GLint id);

		UniformGL4();

		virtual ~UniformGL4();

	protected:
		ContextGL4 * m_context;
		Shader*		 m_shader;
		GLint        m_id;

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
	class ContextGL4 : public Context
	{
	public:

		virtual RenderDriver get_render_driver();
		virtual RenderDriverInfo get_render_driver_info();
		virtual void print_info();

		virtual bool init();
		virtual void close();

		virtual const ClearColorState& get_clear_color_state();
		virtual void set_clear_color_state(const ClearColorState& cf);
		virtual void clear(int type = CLEAR_COLOR_DEPTH);

		virtual const DepthBufferState& get_depth_buffer_state();
		virtual void set_depth_buffer_state(const DepthBufferState& cf);

		virtual const CullfaceState& get_cullface_state();
		virtual void set_cullface_state(const CullfaceState& cf);

		virtual const ViewportState& get_viewport_state();
		virtual void set_viewport_state(const ViewportState& vs);

		virtual const BlendState& get_blend_state();
		virtual void set_blend_state(const BlendState& bs);

		virtual const State& get_render_state();
		virtual void set_render_state(const State& rs);

		//BO
		virtual ConstBuffer* create_stream_CB(const unsigned char* data, size_t size);
		virtual VertexBuffer* create_stream_VBO(const unsigned char* vbo, size_t stride, size_t n);
		virtual IndexBuffer* create_stream_IBO(const unsigned int* ibo, size_t size);

		virtual ConstBuffer* create_CB(const unsigned char* data, size_t size);
		virtual VertexBuffer* create_VBO(const unsigned char* vbo, size_t stride, size_t n);
		virtual IndexBuffer* create_IBO(const unsigned int* ibo, size_t size);

		virtual Variant get_native_CB(const ConstBuffer*);
		virtual Variant get_native_VBO(const VertexBuffer*);
		virtual Variant get_native_IBO(const IndexBuffer*);

		virtual void update_steam_CB(ConstBuffer* cb, const unsigned char* vb, size_t n);
		virtual void update_steam_VBO(VertexBuffer* vbo, const unsigned char* vb, size_t n);
		virtual void update_steam_IBO(IndexBuffer* vbo, const unsigned int* ib, size_t n);

		virtual void bind_CB(ConstBuffer*);
		virtual void bind_VBO(VertexBuffer*);
		virtual void bind_IBO(IndexBuffer*);

		virtual void unbind_CB(ConstBuffer*);
		virtual void unbind_VBO(VertexBuffer*);
		virtual void unbind_IBO(IndexBuffer*);

		virtual unsigned char* map_CB(ConstBuffer*, size_t start, size_t n, MappingType type);
		virtual void unmap_CB(ConstBuffer*);

		virtual unsigned char* map_VBO(VertexBuffer*, size_t start, size_t n, MappingType type);
		virtual void unmap_VBO(VertexBuffer*);

		virtual unsigned int*  map_IBO(IndexBuffer*, size_t start, size_t n, MappingType type);
		virtual void unmap_IBO(IndexBuffer*);

		virtual unsigned char* map_TBO(Texture*, MappingType type);
		virtual void unmap_TBO(Texture*);

		virtual void delete_CB(ConstBuffer*&);
		virtual void delete_VBO(VertexBuffer*&);
		virtual void delete_IBO(IndexBuffer*&);
		//draw
		virtual void draw_arrays(DrawType type, unsigned int n);
		virtual void draw_arrays(DrawType type, unsigned int start, unsigned int size);
		virtual void draw_elements(DrawType type, unsigned int n);

		//IL=Input Layaut
		virtual InputLayout* create_IL(const AttributeList& atl);
		virtual size_t size_IL(const InputLayout* layout);
		virtual bool   has_a_position_IL(const InputLayout* layout);
		virtual size_t position_offset_IL(const InputLayout* layout);
		virtual void delete_IL(InputLayout*&);
		virtual void bind_IL(InputLayout*);
		virtual void unbind_IL(InputLayout* il);

		//depth
		virtual float get_depth(const Vec2& pixel);

		//color
		virtual Vec4 get_color(const Vec2& pixel);

		//texture
		virtual Texture* create_texture
		(
			const TextureRawDataInformation& data,
			const TextureGpuDataInformation& info
		);
		virtual Texture* create_cube_texture
		(
			const TextureRawDataInformation  data[6],
			const TextureGpuDataInformation& info
		);
		virtual  std::vector< unsigned char > get_texture(Texture*, int level = 0);
		virtual void bind_texture(Texture*, int n);
		virtual void unbind_texture(Texture*);
		virtual void unbind_texture(int n);
		virtual void delete_texture(Texture*&);

		//shader
		virtual Shader* create_shader(const std::vector< ShaderSourceInformation >& infos);

		virtual bool shader_compiled_with_errors(Shader* shader);
		virtual bool shader_linked_with_error(Shader* shader);
		virtual std::vector< std::string > get_shader_compiler_errors(Shader* shader);
		virtual std::string get_shader_liker_error(Shader* shader);

		virtual void bind_shader(Shader* shader);
		virtual void unbind_shader(Shader* shader);

		virtual void delete_shader(Shader*&);

		virtual Shader*  get_bind_shader();
		virtual Uniform* get_uniform(Shader*, const std::string& uname);

		//target
		virtual Target* create_render_target(const std::vector< TargetField >& textures);
		virtual void enable_render_target(Target*);
		virtual void disable_render_target(Target*);
		virtual void delete_render_target(Target*&);
		//copy target
		virtual void copy_target_to_target(
			const IVec4& from_area,
			Target* from,
			const IVec4& to_area,
			Target* to,
			TargetType	mask
		);

		//debug
		virtual bool print_errors();
		//Output file name and line
		virtual bool print_errors(const char* source_file_name, int line);

		//context
		BindContext        s_bind_context;
		State			   s_render_state;
		GLuint             s_vao_attributes;
		RenderDriverInfo   s_render_driver_info;
	};
}
}