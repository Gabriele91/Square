//
//  Square
//
//  Created by Gabriele on 15/08/16.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#include "Square/Config.h"
#include "Square/Driver/Render.h"
#include "Square/Driver/OpenGL/OpenGL4.h"
//--------------------------------------------------
#include <iostream>
#include <unordered_map>
//RIGHT HEAND
#define DIRECTX_MODE
//--------------------------------------------------
//MacOS only 4.1 GL
// shader type
#ifndef GL_COMPUTE_SHADER
#define GL_COMPUTE_SHADER   0x91B9
#endif
// texture ops
#ifndef GL_TEXTURE_COMPONENTS
#define GL_TEXTURE_COMPONENTS 0x1003
#endif
// memory ops
#ifndef GL_BUFFER_UPDATE_BARRIER_BIT
#define GL_BUFFER_UPDATE_BARRIER_BIT    0x00000200
#define glMemoryBarrier(__args__)
#endif
// undef GL_R
#ifndef GL_R
#define GL_R GL_RED
#endif
//--------------------------------------------------

namespace Square
{
namespace Render
{
	//TEXTURE
	class Texture
	{
	public:
		GLenum       m_type_texture{ GL_TEXTURE_2D };
		int          m_last_bind{ -1 };
		unsigned int m_tbo{ 0 };

		Texture()  {}

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

	//shader
	class  Shader
	{
	public:
		/////////////////////////////////////////////////////////////////////////
		using uniform_map = std::unordered_map< std::string, Uniform >;
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
		uniform_map m_uniform_map;
		long m_uniform_ntexture{ -1 }; //n texture bind

		//context
		unsigned int m_shader_id{ 0 }; //< shader program
		unsigned int m_shaders[ST_N_SHADER];//< shaders

		//shader compile errors
		struct shader_compile_error
		{
			ShaderType m_type;
			std::string m_log;
		};

		//shaders compiler errors
		std::vector < shader_compile_error > m_errors;

		//linking error
		std::string m_liker_log;

		//add error log
		void push_compiler_error(const shader_compile_error& error_log)
		{
			m_errors.push_back(std::move(error_log));
		}

		void push_liker_error(const std::string& error_log)
		{
			m_liker_log += error_log;
			m_liker_log += "\n";
		}

	};

	const char* Shader::glsl_default_header=
	R"GLSL(
	//POSITION TRANSFORM
	#define ATT_POSITIONT 0
	#define ATT_NORMAL0   1
	#define ATT_TEXCOORD0 2
	#define ATT_TANGENT0  3
	#define ATT_BINORMAL0 4
	#define ATT_COLOR0    5

	//POSITION 0
	#define ATT_POSITION0 6
	#define ATT_NORMAL1   7
	#define ATT_TEXCOORD1 8
	#define ATT_TANGENT1  9
	#define ATT_BINORMAL1 10
	#define ATT_COLOR1    11

	//POSITION 1
	#define ATT_POSITION1 12
	#define ATT_NORMAL2   13
	#define ATT_TEXCOORD2 14
	#define ATT_TANGENT2  15
	#define ATT_BINORMAL2 16
	#define ATT_COLOR2    17
	)GLSL";

	const char* Shader::glsl_header_by_type[ST_N_SHADER] =
	{
		//VERTEX
		"#define cbuffer     layout(std140) uniform     \n"
		"#define saturate(x) clamp( x, 0.0, 1.0 )       \n"
		"#define lerp        mix                        \n",
		//FRAGMENT
		"#define cbuffer     layout(std140) uniform     \n"
		"#define saturate(x) clamp( x, 0.0, 1.0 )       \n"
		"#define lerp        mix                        \n"
		"#define bestp                                  \n"
		"#define highp                                  \n"
		"#define mediump                                \n"
		"#define lowp                                   \n",
		//GEOMETRY
		"#define cbuffer     layout(std140) uniform     \n"
		"#define saturate(x) clamp( x, 0.0, 1.0 )       \n"
		"#define lerp        mix                        \n",
		//TASSELLATION_CONTROL
		"#define cbuffer     layout(std140) uniform     \n"
		"#define saturate(x) clamp( x, 0.0, 1.0 )       \n"
		"#define lerp        mix                        \n",
		//TASSELLATION_EVALUATION
		"#define cbuffer     layout(std140) uniform     \n"
		"#define saturate(x) clamp( x, 0.0, 1.0 )       \n"
		"#define lerp        mix                        \n",
		//COMPUTE
		"#define cbuffer     layout(std140) uniform     \n"
		"#define saturate(x) clamp( x, 0.0, 1.0 )       \n"
		"#define lerp        mix                        \n"
	};

	const char* Shader::glsl_shader_names[ST_N_SHADER] =
	{
		"VERTEX_SHADER",
		"FRAGMENT_SHADER",
		"GEOMETRY_SHADER",
		"TESS_CONTROL_SHADER",
		"TESS_EVALUATION_SHADER",
		"COMPUTE_SHADER"
	};

	GLenum Shader::glsl_type_from_square_type[ST_N_SHADER] =
	{
		GL_VERTEX_SHADER,
		GL_FRAGMENT_SHADER,
		GL_GEOMETRY_SHADER,
		GL_TESS_CONTROL_SHADER,
		GL_TESS_EVALUATION_SHADER,
		GL_COMPUTE_SHADER
	};

	//util_define
	#define uniform_id(shader) (*((GLint*)(&m_data)))
	//uniform
	void Uniform::set_value(Texture* in_texture)
	{
		long n_texture = ++m_shader->m_uniform_ntexture;
		//bind texture
		bind_texture(in_texture, (int)n_texture);
		//bind id
		glUniform1i(uniform_id(m_shader), (int)n_texture);
	}

	void Uniform::set_value(int i)
	{
		glUniform1i(uniform_id(m_shader), i);
	}
	void Uniform::set_value(float f)
	{
		glUniform1f(uniform_id(m_shader), f);
	}
	void Uniform::set_value(const Vec2& v2)
	{
		glUniform2fv(uniform_id(m_shader), 1, value_ptr(v2));
	}
	void Uniform::set_value(const Vec3& v3)
	{
		glUniform3fv(uniform_id(m_shader), 1, value_ptr(v3));
	}
	void Uniform::set_value(const Vec4& v4)
	{
		glUniform4fv(uniform_id(m_shader), 1, value_ptr(v4));
	}
	void Uniform::set_value(const Mat3& m3)
	{
		glUniformMatrix3fv(uniform_id(m_shader), 1, GL_FALSE, value_ptr(m3));
	}
	void Uniform::set_value(const Mat4& m4)
	{
		glUniformMatrix4fv(uniform_id(m_shader), 1, GL_FALSE, value_ptr(m4));
	}

	void Uniform::set_value(const int* i, size_t n)
	{
		glUniform1iv(uniform_id(m_shader), (GLsizei)n, i);
	}
	void Uniform::set_value(const float* f, size_t n)
	{
		glUniform1fv(uniform_id(m_shader), (GLsizei)n, f);
	}
	void Uniform::set_value(const Vec2* v2, size_t n)
	{
		glUniform2fv(uniform_id(m_shader), (GLsizei)n, value_ptr(*v2));
	}
	void Uniform::set_value(const Vec3* v3, size_t n)
	{
		glUniform3fv(uniform_id(m_shader), (GLsizei)n, value_ptr(*v3));
	}
	void Uniform::set_value(const Vec4* v4, size_t n)
	{
		glUniform4fv(uniform_id(m_shader), (GLsizei)n, value_ptr(*v4));
	}
	void Uniform::set_value(const Mat3* m3, size_t n)
	{
		glUniformMatrix3fv(uniform_id(m_shader), (GLsizei)n, GL_FALSE, value_ptr(*m3));
	}
	void Uniform::set_value(const Mat4* m4, size_t n)
	{
		glUniformMatrix4fv(uniform_id(m_shader), (GLsizei)n, GL_FALSE, value_ptr(*m4));
	}

	void Uniform::set_value(const std::vector < int >& i)
	{
		set_value(i.data(), i.size());
	}
	void Uniform::set_value(const std::vector < float >& f)
	{
		set_value(f.data(), f.size());
	}
	void Uniform::set_value(const std::vector < Vec2 >& v2)
	{
		set_value(v2.data(), v2.size());
	}
	void Uniform::set_value(const std::vector < Vec3 >& v3)
	{
		set_value(v3.data(), v3.size());
	}
	void Uniform::set_value(const std::vector < Vec4 >& v4)
	{
		set_value(v4.data(), v4.size());
	}
	void Uniform::set_value(const std::vector < Mat3 >& m3)
	{
		set_value(m3.data(), m3.size());
	}
	void Uniform::set_value(const std::vector < Mat4 >& m4)
	{
		set_value(m4.data(), m4.size());
	}

	void Uniform::set_value(const ConstBuffer* buffer)
	{
		glUniformBlockBinding(GL_UNIFORM_BUFFER, (GLuint)m_shader->m_shader_id, get_native_CB(buffer).get<GLuint>());
	}

	bool Uniform::is_valid(){ return m_shader && uniform_id(m_shader) != -1; }
	
	Shader* Uniform::get_shader()
	{
		return m_shader;
	}

	Uniform::Uniform(Shader* shader, void* data)
	:m_shader(shader)
	,m_data(data)
	{
	}
	Uniform::~Uniform()
	{

	}

	//BUFFER
	class context_buffer
	{
	public:

		GLuint m_id_buffer;

		context_buffer(GLuint id = 0) :m_id_buffer(id) {}

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

	class ConstBuffer : public context_buffer
	{
	public:
		ConstBuffer(GLuint id = 0) :context_buffer(id) {}
	};

	class VertexBuffer : public context_buffer
	{
	public:
		VertexBuffer(GLuint id = 0) :context_buffer(id) {}
	};

	class IndexBuffer : public context_buffer
	{
	public:
		IndexBuffer(GLuint id = 0) :context_buffer(id) {}
	};

	//INPUT LAYOUT
	class InputLayout
	{
	public:
		AttributeList m_list;
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
    ////////////////////
    struct bind_context
    {
        Texture*       m_textures[32]  { nullptr };
		Texture*	   m_texture_buffer{ nullptr };
		ConstBuffer*  m_const_buffer  { nullptr };
        VertexBuffer* m_vertex_buffer { nullptr };
        IndexBuffer*  m_index_buffer  { nullptr };
        InputLayout*  m_input_layout  { nullptr };
        Target* m_render_target { nullptr };
		Shader*		   m_shader		   { nullptr };
    };
    
    static int make_test_to_get_shader_version()
    {
        struct shader_test
        {
            int m_version;
            const char* m_shader;
        };
        #define shader_test(x)\
        {\
          x,\
          "#version " #x "\n"\
          "void main(){}"\
        },
        shader_test tests[]=
        {
            shader_test(100) //fake, no test
            //OpenGL 2->3.2
            shader_test(110)
            shader_test(120)
            shader_test(130)
            shader_test(140)
            shader_test(150)
            //OpenGL 3.3
            shader_test(330)
            //OpenGL 4.x
            shader_test(400)
            shader_test(410)
            shader_test(420)
            shader_test(430)
            shader_test(440)
            shader_test(450)
        };
        //id test
        int   i=0;
        int   supported=0;
        GLint is_compiled = GL_TRUE;
        //tests
        while(i < (sizeof(tests)/sizeof(shader_test)-1))
        {
            //alloc
            const GLuint shader = glCreateShader(GL_VERTEX_SHADER);
            //compile
            if (shader)
            {
                //next
                ++i;
                //test
                glShaderSource(shader, 1, &(tests[i].m_shader), NULL);
                glCompileShader(shader);
                glGetShaderiv(shader, GL_COMPILE_STATUS, &is_compiled);
                glDeleteShader(shader);
                //test
                if(is_compiled == GL_TRUE) supported=i;
            }
            else
            {
                std::cout << "Render driver error, can't determine supported shader version" << std::endl;
                break;
            }
        }
        //undef
        #undef shader_test
        //output
        return tests[supported].m_version;
    }
    
    ////////////////////
	//     RENDER     //
	////////////////////

	///////////////////////
	//globals
    bind_context       s_bind_context;
	State       s_render_state;
	GLuint             s_vao_attributes;
    RenderDriverInfo s_render_driver_info;
	///////////////////////
        
    static void compute_render_driver_info()
    {
        RenderDriverInfo m_info;
        //type
        s_render_driver_info.m_render_driver = DR_OPENGL;
        //Get driver vendor
        s_render_driver_info.m_name = (const char*)glGetString(GL_VENDOR);
        //get version
        glGetIntegerv(GL_MAJOR_VERSION, &s_render_driver_info.m_major_version);
        glGetIntegerv(GL_MINOR_VERSION, &s_render_driver_info.m_minor_version);
        //shader type
        s_render_driver_info.m_shader_language = "GLSL";
        //shader version
        s_render_driver_info.m_shader_version  = make_test_to_get_shader_version();
    }
        
	bool init()
	{

#ifdef _WIN32
		//enable glew experimental (OpenGL3/4)
		glewExperimental = GL_TRUE;
		//try to init glew (get OpenGL calls)
		if (glewInit() != GLEW_OK)
		{
			std::cout << "Glew init fail" << std::endl;
			return false;
		}
		//clear OpenGL error by Glew init
		else while ((glGetError()) != GL_NO_ERROR);
#endif

#if defined(DIRECTX_MODE)
		//DirectX like topology
		glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
		//DirectX like z buffer
		glDepthRange(0.0f, 1.0f);
		//Front face
		glFrontFace(GL_CW);
#endif
		//clean
		print_errors();
        //get info
        compute_render_driver_info();
        //clean
        print_errors();
		//attributes vao
		glGenVertexArrays(1, &s_vao_attributes);
		glBindVertexArray(s_vao_attributes);
		//set default state
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glDisable(GL_BLEND);
		glViewport(0, 0, 0, 0);
		glClearColor(GLclampf(0.1), GLclampf(0.5), GLclampf(1.0), GLclampf(1.0));
		//save cullface state
		s_render_state.m_cullface.m_cullface = CF_BACK;
		//save depth state
		s_render_state.m_depth.m_mode = DM_ENABLE_AND_WRITE;
		s_render_state.m_depth.m_type = DT_LESS;
		//save blend state
		s_render_state.m_blend.m_enable = false;
		s_render_state.m_blend.m_src = BLEND_ONE;
		s_render_state.m_blend.m_dst = BLEND_ZERO;
		//save viewport state
		s_render_state.m_viewport.m_viewport = Vec4(0, 0, 0, 0);
		//save clear color state
		s_render_state.m_clear_color.m_color = Vec4(0.1, 0.5, 1.0, 1.0);
		//clear all errors
		print_errors();
		//return
		return true;
	}

	void print_info()
	{
		std::string renderer = (const char*)glGetString(GL_RENDERER);
		std::string version = (const char*)glGetString(GL_VERSION);
		std::cout << "Renderer: " << renderer << std::endl;
		std::cout << "OpenGL version supported: " << version << std::endl;
	}

	void close()
	{
		if (s_vao_attributes) glDeleteVertexArrays(1, &s_vao_attributes);
	}

	RenderDriver get_render_driver()
	{
		return s_render_driver_info.m_render_driver;
	}
        
    RenderDriverInfo get_render_driver_info()
    {
        return s_render_driver_info;
    }

	const ClearColorState& get_clear_color_state()
	{
		return s_render_state.m_clear_color;
	}
        
	void set_clear_color_state(const ClearColorState& clear_color)
	{
		if (s_render_state.m_clear_color != clear_color)
		{
			s_render_state.m_clear_color = clear_color;
			glClearColor(clear_color.m_color.r, clear_color.m_color.g, clear_color.m_color.b, clear_color.m_color.a);
		}
	}

	void clear(int type)
	{
        GLbitfield clear_type_gl = 0;
        if(type & CLEAR_COLOR) clear_type_gl |= GL_COLOR_BUFFER_BIT;
        if(type & CLEAR_DEPTH) clear_type_gl |= GL_DEPTH_BUFFER_BIT;
		if(clear_type_gl) glClear(clear_type_gl);
	}

	const DepthBufferState& get_depth_buffer_state()
	{
		return s_render_state.m_depth;
	}

	static GLenum get_depth_func(DepthFuncType type)
	{
		switch (type)
		{
		default:
		case DT_LESS:         return GL_LESS;
		case DT_NEVER:        return GL_NEVER;
		case DT_EQUAL:        return GL_EQUAL;
		case DT_GREATER:      return GL_GREATER;
		case DT_NOT_EQUAL:    return GL_NOTEQUAL;
		case DT_LESS_EQUAL:   return GL_LEQUAL;
		case DT_GREATER_EQUAL:return GL_GEQUAL;
		case DT_ALWAYS:       return GL_ALWAYS;
		}
	}

	void set_depth_buffer_state(const DepthBufferState& depth)
	{

		if (s_render_state.m_depth != depth)
		{
			s_render_state.m_depth = depth;
			switch (s_render_state.m_depth.m_mode)
			{
			case DepthMode::DM_DISABLE:
				glDisable(GL_DEPTH_TEST);
			break;
			case DepthMode::DM_ENABLE_AND_WRITE:
				glEnable(GL_DEPTH_TEST);
				glDepthMask(true);
			break;
			case DepthMode::DM_ENABLE_ONLY_READ:
				glEnable(GL_DEPTH_TEST);
				glDepthMask(false);
			break;
			default: break;
			}
			glDepthFunc(get_depth_func(depth.m_type));
		}
	}

	const CullfaceState& get_cullface_state()
	{
		return s_render_state.m_cullface;
	}

	void set_cullface_state(const CullfaceState& cfs)
	{

		if (s_render_state.m_cullface != cfs)
		{
			s_render_state.m_cullface = cfs;

			if (cfs.m_cullface == CF_DISABLE)
			{
				glDisable(GL_CULL_FACE);
			}
			else
			{
				glEnable(GL_CULL_FACE);
				switch (cfs.m_cullface)
				{
				case CF_BACK:           glCullFace(GL_BACK);           break;
				case CF_FRONT:          glCullFace(GL_FRONT);          break;
				case CF_FRONT_AND_BACK: glCullFace(GL_FRONT_AND_BACK); break;
				default: break;
				}
			}
		}
	}

	const ViewportState& get_viewport_state()
	{
		//return
		return s_render_state.m_viewport;
	}

	void  set_viewport_state(const ViewportState& vs)
	{
		if (s_render_state.m_viewport != vs)
		{
			s_render_state.m_viewport = vs;
			glViewport(
				(GLint)vs.m_viewport.x, 
				(GLint)vs.m_viewport.y, 
				(GLsizei)vs.m_viewport.z, 
				(GLsizei)vs.m_viewport.w
			);
		}
	}

	const BlendState& get_blend_state()
	{
		return s_render_state.m_blend;
	}

	static GLenum get_blend_type(BlendType type)
	{
		switch (type)
		{
		default:
		case BLEND_ZERO: return GL_ZERO;
		case BLEND_ONE:  return GL_ONE;

		case BLEND_ONE_MINUS_DST_COLOR: return GL_ONE_MINUS_DST_COLOR;
		case BLEND_ONE_MINUS_DST_ALPHA: return GL_ONE_MINUS_DST_ALPHA;
		case BLEND_ONE_MINUS_SRC_COLOR: return GL_ONE_MINUS_SRC_COLOR;
		case BLEND_ONE_MINUS_SRC_ALPHA: return GL_ONE_MINUS_SRC_ALPHA;

		case BLEND_DST_COLOR: return GL_DST_COLOR;
		case BLEND_DST_ALPHA: return GL_DST_ALPHA;

		case BLEND_SRC_COLOR: return GL_SRC_COLOR;
		case BLEND_SRC_ALPHA: return GL_SRC_ALPHA;
		case BLEND_SRC_ALPHA_SATURATE: return GL_SRC_ALPHA_SATURATE;
		}
	}

	void  set_blend_state(const BlendState& bs)
	{
		if (s_render_state.m_blend != bs)
		{
			//save blend
			s_render_state.m_blend = bs;
			//set state
			if (bs.m_enable)
			{
				glEnable(GL_BLEND);
				glBlendFunc(get_blend_type(bs.m_src), get_blend_type(bs.m_dst));
			}
			else
			{
				glBlendFunc(GL_ONE, GL_ZERO);
				glDisable(GL_BLEND);
				//save blend
				s_render_state.m_blend.m_src = BLEND_ONE;
				s_render_state.m_blend.m_dst = BLEND_ZERO;
				s_render_state.m_blend.m_enable = false;
			}
		}
	}

	const State& get_render_state()
	{
		return s_render_state;
	}

	void set_render_state(const State& rs)
	{
		set_clear_color_state(rs.m_clear_color);
		set_cullface_state(rs.m_cullface);
		set_viewport_state(rs.m_viewport);
		set_depth_buffer_state(rs.m_depth);
		set_blend_state(rs.m_blend);
	}

	//projection matrix
	/*
	VBO & IBO
	*/
	ConstBuffer* create_stream_CB(const unsigned char* data, size_t size)
	{
		auto ptr = new ConstBuffer();
		ptr->gen_buffer();
		glBindBuffer(GL_UNIFORM_BUFFER, *ptr);
		glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STREAM_DRAW);
		return ptr;
	}

	VertexBuffer* create_stream_VBO(const unsigned char* vbo, size_t stride, size_t n)
	{
		auto ptr = new VertexBuffer();
		ptr->gen_buffer();
		glBindBuffer(GL_ARRAY_BUFFER, *ptr);
		glBufferData(GL_ARRAY_BUFFER, stride*n, vbo, GL_STREAM_DRAW);
		return ptr;
	}

	IndexBuffer* create_stream_IBO(const unsigned int* ibo, size_t size)
	{
		auto ptr = new IndexBuffer();
		ptr->gen_buffer();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ptr);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*size, ibo, GL_STREAM_DRAW);
		return ptr;
	}
		
	ConstBuffer* create_CB(const unsigned char* data, size_t size)
	{
		auto ptr = new ConstBuffer();
		ptr->gen_buffer();
		glBindBuffer(GL_UNIFORM_BUFFER, *ptr);
		glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STATIC_DRAW);
		return ptr;
	}

	VertexBuffer* create_VBO(const unsigned char* vbo, size_t stride, size_t n)
	{
		auto ptr = new VertexBuffer();
		ptr->gen_buffer();
		glBindBuffer(GL_ARRAY_BUFFER, *ptr);
		glBufferData(GL_ARRAY_BUFFER, stride*n, vbo, GL_STATIC_DRAW);
		return ptr;
	}

	IndexBuffer* create_IBO(const unsigned int* ibo, size_t size)
	{
		auto ptr = new IndexBuffer();
		ptr->gen_buffer();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ptr);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*size, ibo, GL_STATIC_DRAW);
		return ptr;
	}

	Variant get_native_CB(const ConstBuffer* cb)
	{
		return{ cb->m_id_buffer };
	}

	Variant get_native_VBO(const VertexBuffer* vb)
	{
		return{ vb->m_id_buffer };
	}

	Variant get_native_IBO(const IndexBuffer* ib)
	{
		return{ ib->m_id_buffer };
	}

	void update_steam_CB(ConstBuffer* cb, const unsigned char* data, size_t size)
	{
		//get state
		GLint lastbind = 0;
		glGetIntegerv(GL_UNIFORM_BUFFER, &lastbind);
		//change
		glBindBuffer(GL_UNIFORM_BUFFER, *cb);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
		//restore
		glBindBuffer(GL_UNIFORM_BUFFER, lastbind);
	}

	void update_steam_VBO(VertexBuffer* vbo, const unsigned char* data, size_t size)
	{
		//get state
		GLint lastbind = 0;
		glGetIntegerv(GL_ARRAY_BUFFER, &lastbind);
		//change
		glBindBuffer(GL_ARRAY_BUFFER, *vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
		//restore
		glBindBuffer(GL_ARRAY_BUFFER, lastbind);
	}

	void update_steam_IBO(IndexBuffer* ibo, const unsigned int* data, size_t size) {
		//get state
		GLint lastbind;
		glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER, &lastbind);
		//change
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ibo);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, size * sizeof(unsigned int), data);
		//restore
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lastbind);
	}
		 
	void bind_CB(ConstBuffer* cb)
	{
		if (cb && s_bind_context.m_const_buffer != cb)
		{
			//unbind
			if (s_bind_context.m_const_buffer)
			{
				unbind_CB(s_bind_context.m_const_buffer);
			}
			//bind
			glBindBuffer(GL_UNIFORM_BUFFER, *cb);
			//update
			s_bind_context.m_const_buffer = cb;
		}
	}

	void bind_VBO(VertexBuffer* vbo)
	{
        if(vbo && s_bind_context.m_vertex_buffer != vbo)
        {
            //unbind
            if(s_bind_context.m_vertex_buffer)
            {
                unbind_VBO(s_bind_context.m_vertex_buffer);
            }
            //bind
            glBindBuffer(GL_ARRAY_BUFFER, *vbo);
            //update
            s_bind_context.m_vertex_buffer = vbo;
        }
	}

	void bind_IBO(IndexBuffer* ibo)
    {
        if(ibo && s_bind_context.m_index_buffer != ibo)
        {
            //unbind
            if(s_bind_context.m_index_buffer)
            {
                unbind_IBO(s_bind_context.m_index_buffer);
            }
            //bind
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ibo);
            //update
            s_bind_context.m_index_buffer = ibo;
        }
	}

	void unbind_CB(ConstBuffer* cb)
	{
		if (cb)
		{
			assert(s_bind_context.m_const_buffer == cb);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
			s_bind_context.m_const_buffer = nullptr;
		}
	}

	void unbind_VBO(VertexBuffer* vbo)
	{
        if(vbo)
        {
            assert(s_bind_context.m_vertex_buffer == vbo);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            s_bind_context.m_vertex_buffer = nullptr;
        }
	}

	void unbind_IBO(IndexBuffer* ibo)
    {
        if(ibo)
        {
            assert(s_bind_context.m_index_buffer == ibo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            s_bind_context.m_index_buffer = nullptr;
        }
	}


	GLbitfield get_mapping_type_map_buff_range(MappingType type)
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
		case MAP_WRITE_AND_READ: return GL_MAP_WRITE_BIT | GL_MAP_READ_BIT /*| GL_MAP_UNSYNCHRONIZED_BIT*/; break;
		case MAP_WRITE: return GL_MAP_WRITE_BIT /*| GL_MAP_UNSYNCHRONIZED_BIT*/;
		case MAP_READ:  return GL_MAP_READ_BIT /*| GL_MAP_UNSYNCHRONIZED_BIT*/;
		}
	}

	GLbitfield get_mapping_type_map_buff(MappingType type)
	{
		switch (type)
		{
		default:
		case MAP_WRITE_AND_READ: return GL_READ_WRITE; break;
		case MAP_WRITE: return GL_WRITE_ONLY ;
		case MAP_READ:  return GL_READ_ONLY;
		}
	}

	unsigned char* map_CB(ConstBuffer* cb, size_t start, size_t n, MappingType type)
	{
		bind_CB(cb);
		glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
		return (unsigned char*)glMapBufferRange(GL_UNIFORM_BUFFER, start, n, get_mapping_type_map_buff_range(type));
	}

	void unmap_CB(ConstBuffer* cb)
	{
		glUnmapBuffer(GL_UNIFORM_BUFFER);
		unbind_CB(cb);
	}
		
	unsigned char* map_VBO(VertexBuffer* vbo, size_t start, size_t n, MappingType type)
	{
		bind_VBO(vbo);
		glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
		return (unsigned char*)glMapBufferRange(GL_ARRAY_BUFFER, start, n, get_mapping_type_map_buff_range(type));
	}

	void unmap_VBO(VertexBuffer* vbo)
	{
		glUnmapBuffer(GL_ARRAY_BUFFER);
		unbind_VBO(vbo);
	}

	unsigned int*  map_IBO(IndexBuffer* ibo, size_t start, size_t n, MappingType type)
	{
		bind_IBO(ibo);
		glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
		return (unsigned int*)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, start * sizeof(unsigned int), n * sizeof(unsigned int), get_mapping_type_map_buff_range(type));
	}

	void unmap_IBO(IndexBuffer* ibo)
	{
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
		unbind_IBO(ibo);
	}

	unsigned char* map_TBO(Texture* tbo, MappingType type)
	{
		if (tbo && s_bind_context.m_texture_buffer != tbo)
		{
			//unbind / unmap
			if (s_bind_context.m_texture_buffer)
			{
				unmap_TBO(s_bind_context.m_texture_buffer);
			}
			//bind
			glBindBuffer(GL_TEXTURE_BUFFER, tbo->m_tbo);
			//update
			s_bind_context.m_texture_buffer = tbo;
			//wait
			glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
			//map
			return (unsigned char*)glMapBuffer(GL_TEXTURE_BUFFER, get_mapping_type_map_buff(type));
		}
        return nullptr;
	}

	void unmap_TBO(Texture* tbo)
	{
		if (tbo)
		{
			assert(s_bind_context.m_texture_buffer == tbo);
			//unmap
			glUnmapBuffer(GL_TEXTURE_BUFFER);
			//unbind
			glBindBuffer(GL_TEXTURE_BUFFER, 0);
			//update
			s_bind_context.m_texture_buffer = nullptr;
		}
	}

	void delete_CB(ConstBuffer*& cb)
	{
		//test
		if (s_bind_context.m_const_buffer == cb)
		{
			unbind_CB(s_bind_context.m_const_buffer);
		}
		//safe delete
		glDeleteBuffers(1, *cb);
		delete cb;
		cb = nullptr;
	}

	void delete_VBO(VertexBuffer*& vbo)
	{
        //test
        if(s_bind_context.m_vertex_buffer == vbo)
        {
            unbind_VBO(s_bind_context.m_vertex_buffer);
        }
        //safe delete
		glDeleteBuffers(1, *vbo);
		delete vbo;
		vbo = nullptr;
	}

	void delete_IBO(IndexBuffer*& ibo)
    {
        //test
        if(s_bind_context.m_index_buffer == ibo)
        {
            unbind_IBO(s_bind_context.m_index_buffer);
        }
        //safe delete
		glDeleteBuffers(1, *ibo);
		delete ibo;
		ibo = nullptr;
	}
	/*
		Draw
		*/
	inline static GLuint get_draw_type(DrawType type)
	{
		switch (type)
		{
		case DRAW_POINTS:         return GL_POINTS; break;
		case DRAW_LINES:          return GL_LINES;  break;
		case DRAW_LINE_LOOP:      return GL_LINE_LOOP;  break;
		case DRAW_TRIANGLES:      return GL_TRIANGLES; break;
		case DRAW_TRIANGLE_STRIP: return GL_TRIANGLE_STRIP;  break;
		default: return 0; break;
		}
	}

	void draw_arrays(DrawType type, unsigned int n)
	{
		glDrawArrays(get_draw_type(type), 0, n);
	}

	void draw_arrays(DrawType type, unsigned int start, unsigned int size)
	{
		glDrawArrays(get_draw_type(type), start, size);
	}

	void draw_elements(DrawType type, unsigned int n)
	{
		glDrawElements(get_draw_type(type), n, GL_UNSIGNED_INT, (void*)NULL);
	}

	/*
		InputLayout
	*/

	InputLayout* create_IL(const AttributeList& atl)
	{
		return new InputLayout{ atl };
	}

	size_t size_IL(const InputLayout* layout)
	{
		return layout->m_list.size();
	}

	bool has_a_position_IL(const InputLayout* layout)
	{

		for (auto& in : layout->m_list)
			if (in.is_position_transform())
				return true;
		return false;
	}

	size_t position_offset_IL(const InputLayout* layout)
	{
		for (auto& in : layout->m_list)
			if (in.is_position_transform())
				return in.m_offset;
		return ~((size_t)0);
	}

	static GLenum type_component(AttributeStripType type)
	{

		switch (type)
		{
		default:
		case AST_FLOAT:  return GL_FLOAT;
		case AST_FLOAT2: return GL_FLOAT;
		case AST_FLOAT3: return GL_FLOAT;
		case AST_FLOAT4: return GL_FLOAT;

		case AST_INT:  return GL_INT;
		case AST_INT2: return GL_INT;
		case AST_INT3: return GL_INT;
		case AST_INT4: return GL_INT;

		case AST_UINT:  return GL_UNSIGNED_INT;
		case AST_UINT2: return GL_UNSIGNED_INT;
		case AST_UINT3: return GL_UNSIGNED_INT;
		case AST_UINT4: return GL_UNSIGNED_INT;

		case AST_TLESS:  return GL_LESS;
		case AST_TLESS2: return GL_LESS;
		case AST_TLESS3: return GL_LESS;
		case AST_TLESS4: return GL_LESS;

		}
	}

	static bool type_component_is_integer(AttributeStripType type)
	{

		switch (type)
		{
		case AST_INT:
		case AST_INT2:
		case AST_INT3:
		case AST_INT4:
		case AST_UINT: 
		case AST_UINT2:
		case AST_UINT3:
		case AST_UINT4: 
			return true;
		default: 
			return false;
		}
	}

	void bind_IL(InputLayout* layout)
	{
        if(layout && s_bind_context.m_input_layout != layout)
        {
            //bind?
            if(s_bind_context.m_input_layout)
            {
                unbind_IL(s_bind_context.m_input_layout);
            }
            //bind
            for (const Attribute& data : layout->m_list)
            {
                glEnableVertexAttribArray(data.m_attribute);
				//types
				if (type_component_is_integer(data.m_strip))
					glVertexAttribIPointer(
						data.m_attribute,
						(GLint)data.components(),
						type_component(data.m_strip),
						(GLuint)layout->m_list.size(),
						((char *)NULL + (data.m_offset))
					);
				else glVertexAttribPointer(
					data.m_attribute,
					(GLint)data.components(),
					type_component(data.m_strip),
					GL_FALSE,
					(GLuint)layout->m_list.size(),
					((char *)NULL + (data.m_offset))
				);
            }
            //save
            s_bind_context.m_input_layout = layout;
        }
	}

	void unbind_IL(InputLayout* layout)
	{
        if(layout)
        {
            //test
            assert(s_bind_context.m_input_layout==layout);
            //unbind
            for (const Attribute& data : layout->m_list)
            {
                glDisableVertexAttribArray(data.m_attribute);
            }
            //safe
            s_bind_context.m_input_layout = nullptr;
        }
	}

	void delete_IL(InputLayout*& il)
	{
		delete  il;
		il = nullptr;
	}

	//DEPTH
	float get_depth(const Vec2& pixel)
	{
		float depth;
		glReadPixels(GLint(pixel.x), GLint(pixel.y), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
		return depth;
	}

	//RGBA
	Vec4 get_color(const Vec2& pixel)
	{
		Vec4 rgba;
		glReadPixels(GLint(pixel.x), GLint(pixel.y), 1, 1, GL_RGBA, GL_FLOAT, &rgba);
		return rgba;
	}
	/*
		Textures
		*/
	inline const GLenum get_texture_format(TextureFormat type)
	{
		switch (type)
		{
			//RGBA
		case TF_RGBA8: return GL_RGBA8; break;
			//uint
		case TF_RGBA16UI: return GL_RGBA16UI; break;
		case TF_RGBA32UI: return GL_RGBA32UI; break;
			//int
		case TF_RGBA16I: return GL_RGBA16I; break;
		case TF_RGBA32I: return GL_RGBA32I; break;
			//float
		case TF_RGBA16F: return GL_RGBA16F; break;
		case TF_RGBA32F: return GL_RGBA32F; break;
			////////////////////
			//RGB
		case TF_RGB8: return GL_RGBA8; break;
			//uint
		case TF_RGB16UI: return GL_RGB16UI; break;
		case TF_RGB32UI: return GL_RGB32UI; break;
			//int
		case TF_RGB16I: return GL_RGB16I; break;
		case TF_RGB32I: return GL_RGB32I; break;
			//float
		case TF_RGB16F: return GL_RGB16F; break;
		case TF_RGB32F: return GL_RGB32F; break;
			////////////////////
			//RG
		case TF_RG8: return GL_RG8; break;
			//uint
		case TF_RG16UI: return GL_RG16UI; break;
		case TF_RG32UI: return GL_RG32UI; break;
			//int
		case TF_RG16I: return GL_RG16I; break;
		case TF_RG32I: return GL_RG32I; break;
			//float
		case TF_RG16F: return GL_RG16F; break;
		case TF_RG32F: return GL_RG32F; break;
			////////////////////
			//R
		case TF_R8: return GL_R8; break;
			//uint
		case TF_R16UI: return GL_R16UI; break;
		case TF_R32UI: return GL_R32UI; break;
			//int
		case TF_R16I: return GL_R16I; break;
		case TF_R32I: return GL_R32I; break;
			//float
		case TF_R16F: return GL_R16F; break;
		case TF_R32F: return GL_R32F; break;
			///////////////////
		case TF_RGB565:  return GL_RGB565;  break;
		case TF_RGB5A1:  return GL_RGB5_A1; break;
		case TF_RGBA4:   return GL_RGBA4;   break;
			///////////////////
			//DEPTH & STENCIL
		case TF_DEPTH16_STENCIL8: return GL_DEPTH24_STENCIL8; break;
		case TF_DEPTH24_STENCIL8: return GL_DEPTH24_STENCIL8; break;
		case TF_DEPTH32_STENCIL8: return GL_DEPTH32F_STENCIL8; break;
			//DEPTH
		case TF_DEPTH_COMPONENT16: return GL_DEPTH_COMPONENT16; break;
		case TF_DEPTH_COMPONENT24: return GL_DEPTH_COMPONENT24; break;
		case TF_DEPTH_COMPONENT32: return GL_DEPTH_COMPONENT32; break;

		default: return GL_ZERO; break;
		}
	}

	inline const GLenum get_texture_type(TextureType type)
	{
		switch (type)
		{
		case TT_R:             return GL_RED;
		case TT_RG:            return GL_RG;
		case TT_RGB:           return GL_RGB;
		case TT_RGBA:          return GL_RGBA;
		case TT_DEPTH:         return GL_DEPTH_COMPONENT;
		case TT_DEPTH_STENCIL: return GL_DEPTH_STENCIL;
		default: return GL_ZERO; break;
		}
	}

    inline const GLenum get_texture_type_format(TextureTypeFormat type)
    {
        switch (type)
        {
            default:
            case TTF_FLOAT:                      return GL_FLOAT;
            case TTF_UNSIGNED_BYTE:              return GL_UNSIGNED_BYTE;
            case TTF_UNSIGNED_SHORT:             return GL_UNSIGNED_SHORT;
            case TTF_UNSIGNED_INT:               return GL_UNSIGNED_INT;
            case TTF_UNSIGNED_INT_24_8:          return GL_UNSIGNED_INT_24_8;
            case TTF_FLOAT_32_UNSIGNED_INT_24_8: return GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
        }
    }

	static GLenum get_texture_min_filter(TextureMinFilterType type)
	{
		switch (type)
		{
		default:
		case TMIN_NEAREST:                return GL_NEAREST;
		case TMIN_NEAREST_MIPMAP_NEAREST: return GL_NEAREST_MIPMAP_NEAREST;
		case TMIN_NEAREST_MIPMAP_LINEAR:  return GL_NEAREST_MIPMAP_LINEAR;
		case TMIN_LINEAR:                return GL_LINEAR;
		case TMIN_LINEAR_MIPMAP_NEAREST: return GL_LINEAR_MIPMAP_NEAREST;
		case TMIN_LINEAR_MIPMAP_LINEAR:  return GL_LINEAR_MIPMAP_LINEAR;
		}
	}

	static GLenum get_texture_mag_filter(TextureMagFilterType type)
	{
		switch (type)
		{
		default:
		case TMAG_NEAREST:  return GL_NEAREST;
		case TMAG_LINEAR:   return GL_LINEAR;
		}
	}

	static GLenum get_texture_edge_type(TextureEdgeType type)
	{
		switch (type)
		{
		default:
		case TEDGE_CLAMP:  return GL_CLAMP_TO_EDGE;
		case TEDGE_REPEAT: return GL_REPEAT;
		}
	}

	Texture* create_texture
	(
		const TextureRawDataInformation& data,
		const TextureGpuDataInformation& info
	)
	{

		//new texture
		Texture* ctx_texture = new Texture();
		//create a texture id
		ctx_texture->create_TBO();
		//format
		GLenum gl_format = get_texture_format(data.m_format);
		GLenum gl_type = get_texture_type(data.m_type);
		GLenum gl_type_format = get_texture_type_format(data.m_type_format);
		//enable texture
		glBindTexture(ctx_texture->m_type_texture, ctx_texture->m_tbo);
		//create texture buffer
		glTexImage2D
		(
			ctx_texture->m_type_texture,
			0,
			gl_format,
			data.m_width,
			data.m_height,
			0,
			gl_type,
			gl_type_format,
			data.m_bytes
		);
		//set filters
		glTexParameteri(ctx_texture->m_type_texture, GL_TEXTURE_MIN_FILTER, get_texture_min_filter(info.m_min_type));
		glTexParameteri(ctx_texture->m_type_texture, GL_TEXTURE_MAG_FILTER, get_texture_mag_filter(info.m_mag_type));
		glTexParameteri(ctx_texture->m_type_texture, GL_TEXTURE_WRAP_S, get_texture_edge_type(info.m_edge_s));
		glTexParameteri(ctx_texture->m_type_texture, GL_TEXTURE_WRAP_T, get_texture_edge_type(info.m_edge_t));
		// Generate mipmaps, by the way.
		if (info.m_build_mipmap) glGenerateMipmap(ctx_texture->m_type_texture);
		//disable texture
		glBindTexture(ctx_texture->m_type_texture, 0);
		//test
		print_errors();
		//return texture
		return ctx_texture;
	}

	Texture* create_cube_texture
	(
		const TextureRawDataInformation data[6],
		const TextureGpuDataInformation& info
	)
	{

		//new texture
		Texture* ctx_texture = new Texture();
		//set type
		ctx_texture->m_type_texture = GL_TEXTURE_CUBE_MAP;
		//create a texture id
		ctx_texture->create_TBO();
		//add texture
		for (int i = 0; i != 6; ++i)
		{
			//format
			GLenum gl_format = get_texture_format(data[i].m_format);
			GLenum gl_type = get_texture_type(data[i].m_type);
			GLenum gl_type_format = get_texture_type_format(data[i].m_type_format);
			//enable texture
			glBindTexture(ctx_texture->m_type_texture, ctx_texture->m_tbo);
			//create texture buffer
			glTexImage2D
			(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				gl_format,
				data[i].m_width,
				data[i].m_height,
				0,
				gl_type,
				gl_type_format,
				data[i].m_bytes
			);
		}
		//set filters
		glTexParameteri(ctx_texture->m_type_texture, GL_TEXTURE_MIN_FILTER, get_texture_min_filter(info.m_min_type));
		glTexParameteri(ctx_texture->m_type_texture, GL_TEXTURE_MAG_FILTER, get_texture_mag_filter(info.m_mag_type));
		glTexParameteri(ctx_texture->m_type_texture, GL_TEXTURE_WRAP_S, get_texture_edge_type(info.m_edge_s));
		glTexParameteri(ctx_texture->m_type_texture, GL_TEXTURE_WRAP_T, get_texture_edge_type(info.m_edge_t));
		glTexParameteri(ctx_texture->m_type_texture, GL_TEXTURE_WRAP_R, get_texture_edge_type(info.m_edge_r));
		// Generate mipmaps, by the way.
		if (info.m_build_mipmap) glGenerateMipmap(ctx_texture->m_type_texture);
		//disable texture
		glBindTexture(ctx_texture->m_type_texture, 0);
		//test
		print_errors();
		//return texture
		return ctx_texture;
	}
        
	std::vector< unsigned char > get_texture(Texture* tex, int level)
	{
		// get last bind
		auto last_texture = s_bind_context.m_textures[0];
		//unbind
		unbind_texture(last_texture);
		// bind texture
		bind_texture(tex, 0);
		// format
		GLint format, components, width, height;
		// get internal format type of GL texture
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPONENTS, &components);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
		// return
		std::vector< unsigned char > output;
		GLint image_ret_format;
		// cases
		switch (components)
		{
		case GL_R8:   output.resize(width*height * 1); image_ret_format = GL_R;     break;
		case GL_RG8:  output.resize(width*height * 2); image_ret_format = GL_RG;    break;
		case GL_RGB8: output.resize(width*height * 3); image_ret_format = GL_RGB;   break;
		case GL_RGBA8:output.resize(width*height * 4); image_ret_format = GL_RGBA;  break;
		default: return output; break;
		}
		// get
		glGetTexImage(GL_TEXTURE_2D, 0, image_ret_format, GL_UNSIGNED_BYTE, (GLbyte*)output.data());
		// unbind
		unbind_texture(tex);
		// bind last 
		bind_texture(last_texture, 0);
		//return
		return output;
	}

	void bind_texture(Texture* ctx_texture, int n)
	{
        if (ctx_texture && ctx_texture != s_bind_context.m_textures[n])
        {
            //enable
            ctx_texture->enable_TBO(n);
            //disable last
            if(s_bind_context.m_textures[n])
                s_bind_context.m_textures[n]->m_last_bind = -1;
            //add this
            s_bind_context.m_textures[n] = ctx_texture;
        }
	}

	void unbind_texture(Texture* ctx_texture)
	{
        if (ctx_texture)
        {
            //to null
            s_bind_context.m_textures[ctx_texture->m_last_bind] = nullptr;
            //disable
            ctx_texture->disable_TBO();
        }
	}
        
    void unbind_texture(int n)
    {
        unbind_texture(s_bind_context.m_textures[n]);
    }
        
	void delete_texture(Texture*& ctx_texture)
    {
        //bind?
        if(ctx_texture->m_last_bind)
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
	static std::string compiler_shader_error_log(unsigned int shader)
	{
		//return
		std::string info_log;
		//get len
		GLint info_len = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_len);
		//print
		if (info_len > 1)
		{
			info_log.resize(info_len + 1, '\0');
			glGetShaderInfoLog(shader, info_len, NULL, (char*)info_log.c_str());
		}
		return info_log;
	}
	static std::string liker_shader_error_log(unsigned int program)
	{
		//return
		std::string info_log;
		//get len
		GLint info_len = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_len);
		//print
		if (info_len > 1)
		{
			info_log.resize(info_len + 1, '\0');
			glGetProgramInfoLog(program, info_len, NULL, (char*)info_log.c_str());
		}
		return info_log;
	}
    
#if 0
	static const char* glsl_type_to_string(GLenum type)
	{
		switch (type)
		{
		case GL_BOOL: return "bool";
		case GL_INT: return "int";
		case GL_FLOAT: return "float";
		case GL_FLOAT_VEC2: return "vec2";
		case GL_FLOAT_VEC3: return "vec3";
		case GL_FLOAT_VEC4: return "vec4";
		case GL_FLOAT_MAT2: return "mat2";
		case GL_FLOAT_MAT3: return "mat3";
		case GL_FLOAT_MAT4: return "mat4";
		case GL_SAMPLER_2D: return "sampler2D";
		case GL_SAMPLER_3D: return "sampler3D";
		case GL_SAMPLER_CUBE: return "samplerCube";
		case GL_SAMPLER_2D_SHADOW: return "sampler2DShadow";
		default: break;
		}
		return "other";
	}
#endif
    
	Shader* create_shader(const std::vector< ShaderSourceInformation >& infos)
	{
		//alloc
		Shader* out_shader = new Shader();
		//shader program
		out_shader->m_shader_id = glCreateProgram();
		//compile
		for (const ShaderSourceInformation& info : infos)
		{
			if (info.m_shader_source.size())
			{
				//get shader
				auto& source_shader = out_shader->m_shaders[info.m_type];
				//create 
				source_shader = glCreateShader(Shader::glsl_type_from_square_type[info.m_type]);
				//start line
				std::string start_line = "#line " + std::to_string(info.m_line) + "\n";
				//source
				const char* sources[] =
				{
					info.m_shader_header.c_str(),
					Shader::glsl_header_by_type[info.m_type],
					Shader::glsl_default_header,
					start_line.c_str(),
					info.m_shader_source.c_str()
				};
				//add source
				glShaderSource(source_shader, 5, sources, 0);
				//compiling
				glCompileShader(source_shader);
				//get status
				{
					//test
					int is_compiled = false;
					glGetShaderiv(source_shader, GL_COMPILE_STATUS, &is_compiled);
					//get error
					if (!is_compiled)
					{
						//save error
						out_shader->push_compiler_error({ info.m_type, compiler_shader_error_log(source_shader) });
						//delete and to null
						glDeleteShader(source_shader); source_shader = 0;
					}
				}
			}
		}
		//attach
		for (auto shader : out_shader->m_shaders)
		{
			if (shader)  glAttachShader(out_shader->m_shader_id, shader);
		}
		//liking
		glLinkProgram(out_shader->m_shader_id);
		//error?
		{
			//status
			int is_linked = false;
			//get link status
			glGetProgramiv(out_shader->m_shader_id, GL_LINK_STATUS, &is_linked);
			//ok?
			if (!is_linked)
			{
				//get error
				out_shader->push_liker_error(liker_shader_error_log(out_shader->m_shader_id));
			}
		}
		//return shader
		return out_shader;
	}

	bool shader_compiled_with_errors(Shader* shader)
	{
		return shader->m_errors.size() != 0;
	}
	bool shader_linked_with_error(Shader* shader)
	{
		return shader->m_liker_log.size() != 0;
	}
	std::vector< std::string > get_shader_compiler_errors(Shader* shader)
	{
		std::vector< std::string > output;
		//state
		ShaderType last_type = ST_N_SHADER;
		//push
		for (auto& error_log : shader->m_errors)
		{
			if (last_type != error_log.m_type)
			{
				output.push_back(Shader::glsl_shader_names[error_log.m_type]);
				last_type = error_log.m_type;
			}
			output.push_back(error_log.m_log);
		}
		return output;
	}
	std::string get_shader_liker_error(Shader* shader)
	{
		return shader->m_liker_log;
	}

	void bind_shader(Shader* shader)
	{
		if (s_bind_context.m_shader)
		{
			unbind_shader(s_bind_context.m_shader);
		}
		//save
		s_bind_context.m_shader = shader;
		//start texture uniform
		shader->m_uniform_ntexture = -1;
		//uniform parogram shaders
		glUseProgram(shader->m_shader_id);
	}
	void unbind_shader(Shader* shader)
	{
		if (shader)
		{
			assert(s_bind_context.m_shader == shader);
			//disable textures
			while (shader->m_uniform_ntexture >= 0)
			{
				unbind_texture((int)shader->m_uniform_ntexture);
				--shader->m_uniform_ntexture;
			}
			//disable program
			glUseProgram(0);
			//to null
			s_bind_context.m_shader = nullptr;
		}
	}
	void delete_shader(Shader*& shader)
	{
		if (s_bind_context.m_shader == shader)
		{
			unbind_shader(shader);
		}
		delete shader;
		shader = nullptr;
	}

	Shader* get_bind_shader()
	{
		return s_bind_context.m_shader;
	}
	Uniform* get_uniform(Shader* shader,const std::string& uname)
	{
		auto uit = shader->m_uniform_map.find(uname);
		//if find
		if (uit != shader->m_uniform_map.end()) return &uit->second;
		//else
		long long uid = glGetUniformLocation(shader->m_shader_id,uname.c_str());
		if (uid < 0) return nullptr;
		//add and return
		return &(shader->m_uniform_map[uname] = Uniform(shader, (void*)uid));
	}

	/*
		FBO
	*/

	static std::string framebuffer_error_to_str(GLenum error)
	{
		switch (error)
		{
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:         return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
			//      case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:         return "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS";
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
		case GL_FRAMEBUFFER_UNSUPPORTED:                   return "GL_FRAMEBUFFER_UNSUPPORTED";
		case GL_FRAMEBUFFER_COMPLETE:                      return "GL_FRAMEBUFFER_COMPLETE";
		default:                                           return "GL_UNKNOW";
		}
	}

	Target* create_render_target(const std::vector< TargetField >& textures)
	{
		bool   depth_attach = false;
		size_t color_count = 0;
		//create FBO
		auto fbo = new Target;
		fbo->gen_FBO();
		//enable
		fbo->enable_FBO();
		//attach
		for (const TargetField& t_field : textures)
		{
			if (t_field.m_type == RT_COLOR)
			{
				glFramebufferTexture(
					GL_FRAMEBUFFER,
					(GLenum)(GL_COLOR_ATTACHMENT0 + (color_count++)),
					t_field.m_texture->m_tbo,
					0
				);
			}
			else if ((!depth_attach) && t_field.m_type == RT_DEPTH)
			{
				glFramebufferTexture(
					GL_FRAMEBUFFER,
					GL_DEPTH_ATTACHMENT,
					t_field.m_texture->m_tbo,
					0
				);
				depth_attach = true;
			}
			else if ((!depth_attach) && t_field.m_type == RT_DEPTH_STENCIL)
			{
				glFramebufferTexture(
					GL_FRAMEBUFFER,
					GL_DEPTH_STENCIL_ATTACHMENT,
					t_field.m_texture->m_tbo,
					0
				);
				depth_attach = true;
			}
		}
		//set buffer to draw
		std::vector < GLenum > m_buffers_to_draw(color_count);
		for (size_t i = 0; i != color_count; ++i) m_buffers_to_draw[i] = (GLenum)(GL_COLOR_ATTACHMENT0 + i);
		glDrawBuffers((GLuint)color_count, m_buffers_to_draw.data());
		//status
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		//test
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << framebuffer_error_to_str(status) << std::endl;
		}
		//disable
		fbo->disable_FBO();
		//return
		return fbo;
	}

	void enable_render_target(Target* r_target)
	{
        if(r_target && s_bind_context.m_render_target != r_target)
        {
            r_target->enable_FBO();
            s_bind_context.m_render_target = r_target;
        }
	}

	void disable_render_target(Target* r_target)
	{
        if(r_target)
        {
            assert(s_bind_context.m_render_target == r_target);
            r_target->disable_FBO();
            s_bind_context.m_render_target = nullptr;
        }
	}

	void delete_render_target(Target*& r_target)
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

	GLenum get_copy_render_target_type(TargetType type)
	{
		switch (type)
		{
		case RT_COLOR:		  return GL_COLOR_BUFFER_BIT;
		case RT_DEPTH:		  return GL_DEPTH_BUFFER_BIT;
		case RT_STENCIL:	  return GL_STENCIL_BUFFER_BIT;
		case RT_DEPTH_STENCIL:
		default: 
			assert(0); 
			return GL_NONE;  
		break;
		}
	}

	void copy_target_to_target(
		const IVec4& from_area,
		Target* from,
		const IVec4& to_area,
		Target* to,
		TargetType	mask
	)
	{
		//bind buffers
		glBindFramebuffer(GL_READ_FRAMEBUFFER, from ? from->m_fbo : 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, to   ? to->m_fbo   : 0);
		//copy
		if (mask != RT_DEPTH_STENCIL)
		{
			glBlitFramebuffer(
				from_area.x, from_area.y, from_area.z, from_area.w,
				to_area.x, to_area.y, to_area.z, to_area.w,
				get_copy_render_target_type(mask),
				GL_NEAREST
			);
		}
		else
		{
			//DEPTH
			glBlitFramebuffer(
				from_area.x, from_area.y, from_area.z, from_area.w,
				to_area.x, to_area.y, to_area.z, to_area.w,
				GL_DEPTH_BUFFER_BIT,
				GL_NEAREST
			);
			//STENCIL
			glBlitFramebuffer(
				from_area.x, from_area.y, from_area.z, from_area.w,
				to_area.x, to_area.y, to_area.z, to_area.w,
				GL_STENCIL_BUFFER_BIT,
				GL_NEAREST
			);

        }
#if 0
        //reset
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
#endif
		//reset
		if(s_bind_context.m_render_target)
			//set old fbo
			s_bind_context.m_render_target->enable_FBO();
		else 
			//default FBO
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	bool print_errors()
	{
		//get all gl errors
		std::string gl_errors = debug_gl_errors_to_string();
		//print
		if (gl_errors.size())
		{
			std::cout << gl_errors << std::endl;
			return true;
		}
		return false;
	}

	//Output file name and line
	bool print_errors(const char* source_file_name, int line)
	{
		//get all gl errors
		std::string gl_errors = debug_gl_errors_to_string_args(source_file_name,line);
		//print
		if (gl_errors.size())
		{
			std::cout << gl_errors << std::endl;
			return true;
		}
		return false;
	}

//end render
};
}
