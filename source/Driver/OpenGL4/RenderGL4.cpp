//
//  Square
//
//  Created by Gabriele on 15/08/16.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#include "RenderGL4.h"
#include "Square/Driver/Window.h"
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
// offeset "VertexAttribPointer"
#define GL_OFFSET_OF(x) ((char *)NULL + (x))
//--------------------------------------------------

namespace Square
{
namespace Render
{
	const char* Shader::glsl_default_header=
	R"GLSL(
	//POSITION TRANSFORM
	#define ATT_POSITION 0

	//POSITION 0
	#define ATT_POSITION0 0
	#define ATT_NORMAL0   1
	#define ATT_TEXCOORD0 2
	#define ATT_TANGENT0  3
	#define ATT_BINORMAL0 4
	#define ATT_COLOR0    5

	//POSITION 1
	#define ATT_POSITION1 6
	#define ATT_NORMAL1   7
	#define ATT_TEXCOORD1 8
	#define ATT_TANGENT1  9
	#define ATT_BINORMAL1 10
	#define ATT_COLOR1    11

	//POSITION 2
	#define ATT_NORMAL2   12
	#define ATT_TEXCOORD2 13
	#define ATT_TANGENT2  14
	#define ATT_BINORMAL2 15
	#define ATT_COLOR2    16
	)GLSL";

	const char* Shader::glsl_header_by_type[ST_N_SHADER] =
	{
		//VERTEX
		"#define saturate(x) clamp( x, 0.0, 1.0 )       \n"
		"#define lerp        mix                        \n",
		//FRAGMENT
		"#define saturate(x) clamp( x, 0.0, 1.0 )       \n"
		"#define lerp        mix                        \n"
		"#define bestp                                  \n"
		"#define highp                                  \n"
		"#define mediump                                \n"
		"#define lowp                                   \n",
		//GEOMETRY
		"#define saturate(x) clamp( x, 0.0, 1.0 )       \n"
		"#define lerp        mix                        \n",
		//TASSELLATION_CONTROL
		"#define saturate(x) clamp( x, 0.0, 1.0 )       \n"
		"#define lerp        mix                        \n",
		//TASSELLATION_EVALUATION
		"#define saturate(x) clamp( x, 0.0, 1.0 )       \n"
		"#define lerp        mix                        \n",
		//COMPUTE
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

    ////////////////////////////////////////////////////////////////////////////////////////////////////
	//uniform
	void UniformGL4::set(Texture* in_texture)
	{
		long n_texture = ++m_shader->m_uniform_ntexture;
		//bind texture
		m_context->bind_texture(in_texture, (int)n_texture, (int)n_texture);
		//bind id
		glUniform1i(m_id, (int)n_texture);
	}

	void UniformGL4::set(int i)
	{
		glUniform1i(m_id, i);
	}
    void UniformGL4::set(float f)
    {
        glUniform1f(m_id, f);
    }
    void UniformGL4::set(double d)
    {
        glUniform1d(m_id, d);
    }
    
    
    void UniformGL4::set(const IVec2& v2)
    {
        glUniform2iv(m_id, 1, value_ptr(v2));
    }
    void UniformGL4::set(const IVec3& v3)
    {
        glUniform3iv(m_id, 1, value_ptr(v3));
    }
    void UniformGL4::set(const IVec4& v4)
    {
        glUniform4iv(m_id, 1, value_ptr(v4));
    }
    
	void UniformGL4::set(const Vec2& v2)
	{
		glUniform2fv(m_id, 1, value_ptr(v2));
	}
	void UniformGL4::set(const Vec3& v3)
	{
		glUniform3fv(m_id, 1, value_ptr(v3));
	}
	void UniformGL4::set(const Vec4& v4)
	{
		glUniform4fv(m_id, 1, value_ptr(v4));
	}
	void UniformGL4::set(const Mat3& m3)
	{
		glUniformMatrix3fv(m_id, 1, GL_FALSE, value_ptr(m3));
	}
	void UniformGL4::set(const Mat4& m4)
	{
		glUniformMatrix4fv(m_id, 1, GL_FALSE, value_ptr(m4));
	}
    
    void UniformGL4::set(const DVec2& v2)
    {
        glUniform2dv(m_id, 1, value_ptr(v2));
    }
    void UniformGL4::set(const DVec3& v3)
    {
        glUniform3dv(m_id, 1, value_ptr(v3));
    }
    void UniformGL4::set(const DVec4& v4)
    {
        glUniform4dv(m_id, 1, value_ptr(v4));
    }
    void UniformGL4::set(const DMat3& m3)
    {
        glUniformMatrix3dv(m_id, 1, GL_FALSE, value_ptr(m3));
    }
    void UniformGL4::set(const DMat4& m4)
    {
        glUniformMatrix4dv(m_id, 1, GL_FALSE, value_ptr(m4));
    }

    void UniformGL4::set(Texture* tvector, size_t n)
    {
        /* 3D texture? */
        //id array
        auto id = m_id;
        //for all
        while(n--)
        {
            //bind
            long n_texture = ++m_shader->m_uniform_ntexture;
            //bind texture
            m_context->bind_texture(tvector++, (int)n_texture, (int)n_texture);
            //bind ids
            glUniform1i(id++, (int)n_texture);
        }
    }
	void UniformGL4::set(const int* i, size_t n)
	{
		glUniform1iv(m_id, (GLsizei)n, i);
	}
    void UniformGL4::set(const float* f, size_t n)
    {
        glUniform1fv(m_id, (GLsizei)n, f);
    }
    void UniformGL4::set(const double* f, size_t n)
    {
        glUniform1dv(m_id, (GLsizei)n, f);
    }
    
    void UniformGL4::set(const IVec2* v2, size_t n)
    {
        glUniform2iv(m_id, (GLsizei)n, value_ptr(*v2));
    }
    void UniformGL4::set(const IVec3* v3, size_t n)
    {
        glUniform3iv(m_id, (GLsizei)n, value_ptr(*v3));
    }
    void UniformGL4::set(const IVec4* v4, size_t n)
    {
        glUniform4iv(m_id, (GLsizei)n, value_ptr(*v4));
    }
    
	void UniformGL4::set(const Vec2* v2, size_t n)
	{
		glUniform2fv(m_id, (GLsizei)n, value_ptr(*v2));
	}
	void UniformGL4::set(const Vec3* v3, size_t n)
	{
		glUniform3fv(m_id, (GLsizei)n, value_ptr(*v3));
	}
	void UniformGL4::set(const Vec4* v4, size_t n)
	{
		glUniform4fv(m_id, (GLsizei)n, value_ptr(*v4));
	}
	void UniformGL4::set(const Mat3* m3, size_t n)
	{
		glUniformMatrix3fv(m_id, (GLsizei)n, GL_FALSE, value_ptr(*m3));
	}
	void UniformGL4::set(const Mat4* m4, size_t n)
	{
		glUniformMatrix4fv(m_id, (GLsizei)n, GL_FALSE, value_ptr(*m4));
	}
    
    void UniformGL4::set(const DVec2* v2, size_t n)
    {
        glUniform2dv(m_id, (GLsizei)n, value_ptr(*v2));
    }
    void UniformGL4::set(const DVec3* v3, size_t n)
    {
        glUniform3dv(m_id, (GLsizei)n, value_ptr(*v3));
    }
    void UniformGL4::set(const DVec4* v4, size_t n)
    {
        glUniform4dv(m_id, (GLsizei)n, value_ptr(*v4));
    }
    void UniformGL4::set(const DMat3* m3, size_t n)
    {
        glUniformMatrix3dv(m_id, (GLsizei)n, GL_FALSE, value_ptr(*m3));
    }
    void UniformGL4::set(const DMat4* m4, size_t n)
    {
        glUniformMatrix4dv(m_id, (GLsizei)n, GL_FALSE, value_ptr(*m4));
    }

    void UniformGL4::set(const std::vector < Texture* >& t)
    {
        set((Texture*)t.data(), t.size());
    }
	void UniformGL4::set(const std::vector < int >& i)
	{
		set(i.data(), i.size());
	}
    void UniformGL4::set(const std::vector < float >& f)
    {
        set(f.data(), f.size());
    }
    void UniformGL4::set(const std::vector < double >& d)
    {
        set(d.data(), d.size());
    }
    
    void UniformGL4::set(const std::vector < IVec2 >& v2)
    {
        set(v2.data(), v2.size());
    }
    void UniformGL4::set(const std::vector < IVec3 >& v3)
    {
        set(v3.data(), v3.size());
    }
    void UniformGL4::set(const std::vector < IVec4 >& v4)
    {
        set(v4.data(), v4.size());
    }
    
	void UniformGL4::set(const std::vector < Vec2 >& v2)
	{
		set(v2.data(), v2.size());
	}
	void UniformGL4::set(const std::vector < Vec3 >& v3)
	{
		set(v3.data(), v3.size());
	}
	void UniformGL4::set(const std::vector < Vec4 >& v4)
	{
		set(v4.data(), v4.size());
	}
	void UniformGL4::set(const std::vector < Mat3 >& m3)
	{
		set(m3.data(), m3.size());
	}
	void UniformGL4::set(const std::vector < Mat4 >& m4)
	{
		set(m4.data(), m4.size());
	}
    
    void UniformGL4::set(const std::vector < DVec2 >& v2)
    {
        set(v2.data(), v2.size());
    }
    void UniformGL4::set(const std::vector < DVec3 >& v3)
    {
        set(v3.data(), v3.size());
    }
    void UniformGL4::set(const std::vector < DVec4 >& v4)
    {
        set(v4.data(), v4.size());
    }
    void UniformGL4::set(const std::vector < DMat3 >& m3)
    {
        set(m3.data(), m3.size());
    }
    void UniformGL4::set(const std::vector < DMat4 >& m4)
    {
        set(m4.data(), m4.size());
    }
	bool UniformGL4::is_valid(){ return m_context && m_shader && m_id != -1; }
	
	Shader* UniformGL4::get_shader()
	{
		return m_shader;
	}

	UniformGL4::UniformGL4(ContextGL4* context, Shader* shader, GLint id)
	:m_context(context)
	,m_shader(shader)
	,m_id(id)
	{
	}

	UniformGL4::UniformGL4()
	:m_context(nullptr)
	,m_shader(nullptr)
	,m_id(-1)
	{
	}
	
	UniformGL4::~UniformGL4()
	{
		//none
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	//uniform
	void UniformGLGUBO::set(Texture* in_texture)
	{
		long n_texture = ++m_shader->m_uniform_ntexture;
		//bind texture
		m_context->bind_texture(in_texture, (int)n_texture, 0);
		//bind id
		set((int)n_texture);
	}

	void UniformGLGUBO::set(int i)
	{
		m_shader->global_buffer_bind();
		primitive_write(i);
	}
	void UniformGLGUBO::set(float f)
	{
		m_shader->global_buffer_bind();
		primitive_write(f);
	}
	void UniformGLGUBO::set(double d)
	{
		m_shader->global_buffer_bind();
		primitive_write(d);
	}


	void UniformGLGUBO::set(const IVec2& v2)
	{
		m_shader->global_buffer_bind();
		array_write<int>(value_ptr(v2), 2);
	}
	void UniformGLGUBO::set(const IVec3& v3)
	{
		array_write<int>(value_ptr(v3), 3);
	}
	void UniformGLGUBO::set(const IVec4& v4)
	{
		m_shader->global_buffer_bind();
		array_write<int>(value_ptr(v4), 4);
	}

	void UniformGLGUBO::set(const Vec2& v2)
	{
		m_shader->global_buffer_bind();
		array_write(value_ptr(v2), 2);
	}
	void UniformGLGUBO::set(const Vec3& v3)
	{
		m_shader->global_buffer_bind();
		array_write(value_ptr(v3), 3);
	}
	void UniformGLGUBO::set(const Vec4& v4)
	{
		m_shader->global_buffer_bind();
		array_write<float>(value_ptr(v4), 4);
	}
	void UniformGLGUBO::set(const Mat3& m3)
	{
		m_shader->global_buffer_bind();
		array_write<float>(value_ptr(m3), 3 * 3);
	}
	void UniformGLGUBO::set(const Mat4& m4)
	{
		m_shader->global_buffer_bind();
		array_write<float>(value_ptr(m4), 4 * 4);
	}

	void UniformGLGUBO::set(const DVec2& v2)
	{
		m_shader->global_buffer_bind();
		array_write<double>(value_ptr(v2), 2);
	}
	void UniformGLGUBO::set(const DVec3& v3)
	{
		m_shader->global_buffer_bind();
		array_write<double>(value_ptr(v3), 3);
	}
	void UniformGLGUBO::set(const DVec4& v4)
	{
		m_shader->global_buffer_bind();
		array_write<double>(value_ptr(v4), 4);
	}
	void UniformGLGUBO::set(const DMat3& m3)
	{
		m_shader->global_buffer_bind();
		array_write<double>(value_ptr(m3), 3 * 3);
	}
	void UniformGLGUBO::set(const DMat4& m4)
	{
		m_shader->global_buffer_bind();
		array_write<double>(value_ptr(m4), 4 * 4);
	}

	void UniformGLGUBO::set(Texture* tvector, size_t n)
	{
		/* 3D texture? */
		//for all
		while (n--)
		{
			//bind
			long n_texture = ++m_shader->m_uniform_ntexture;
			//bind texture
			m_context->bind_texture(tvector++, (int)n_texture, (int)n_texture);
		}
	}

	void UniformGLGUBO::set(const int* i, size_t n)
	{
		m_shader->global_buffer_bind();
		array_write(i, n);
	}
	void UniformGLGUBO::set(const float* f, size_t n)
	{
		m_shader->global_buffer_bind();
		array_write(f, n);
	}
	void UniformGLGUBO::set(const double* d, size_t n)
	{
		m_shader->global_buffer_bind();
		array_write(d, n);
	}

	void UniformGLGUBO::set(const IVec2* v2, size_t n)
	{
		m_shader->global_buffer_bind();
		array_write(v2, n);
	}
	void UniformGLGUBO::set(const IVec3* v3, size_t n)
	{
		m_shader->global_buffer_bind();
		array_write(v3, n);
	}
	void UniformGLGUBO::set(const IVec4* v4, size_t n)
	{
		m_shader->global_buffer_bind();
		array_write(v4, n);
	}

	void UniformGLGUBO::set(const Vec2* v2, size_t n)
	{
		m_shader->global_buffer_bind();
		array_write(v2, n);
	}
	void UniformGLGUBO::set(const Vec3* v3, size_t n)
	{
		m_shader->global_buffer_bind();
		array_write(v3, n);
	}
	void UniformGLGUBO::set(const Vec4* v4, size_t n)
	{
		m_shader->global_buffer_bind();
		array_write(v4, n);
	}
	void UniformGLGUBO::set(const Mat3* m3, size_t n)
	{
		m_shader->global_buffer_bind();
		array_write(m3, n);
	}
	void UniformGLGUBO::set(const Mat4* m4, size_t n)
	{
		m_shader->global_buffer_bind();
		array_write(m4, n);
	}

	void UniformGLGUBO::set(const DVec2* v2, size_t n)
	{
		m_shader->global_buffer_bind();
		array_write(v2, n);
	}
	void UniformGLGUBO::set(const DVec3* v3, size_t n)
	{
		m_shader->global_buffer_bind();
		array_write(v3, n);
	}
	void UniformGLGUBO::set(const DVec4* v4, size_t n)
	{
		m_shader->global_buffer_bind();
		array_write(v4, n);
	}
	void UniformGLGUBO::set(const DMat3* m3, size_t n)
	{
		m_shader->global_buffer_bind();
		array_write(m3, n);
	}
	void UniformGLGUBO::set(const DMat4* m4, size_t n)
	{
		m_shader->global_buffer_bind();
		array_write(m4, n);
	}

	void UniformGLGUBO::set(const std::vector < Texture* >& t)
	{
		set((Texture*)t.data(), t.size());
	}
	void UniformGLGUBO::set(const std::vector < int >& i)
	{
		set(i.data(), i.size());
	}
	void UniformGLGUBO::set(const std::vector < float >& f)
	{
		set(f.data(), f.size());
	}
	void UniformGLGUBO::set(const std::vector < double >& d)
	{
		set(d.data(), d.size());
	}

	void UniformGLGUBO::set(const std::vector < IVec2 >& v2)
	{
		set(v2.data(), v2.size());
	}
	void UniformGLGUBO::set(const std::vector < IVec3 >& v3)
	{
		set(v3.data(), v3.size());
	}
	void UniformGLGUBO::set(const std::vector < IVec4 >& v4)
	{
		set(v4.data(), v4.size());
	}

	void UniformGLGUBO::set(const std::vector < Vec2 >& v2)
	{
		set(v2.data(), v2.size());
	}
	void UniformGLGUBO::set(const std::vector < Vec3 >& v3)
	{
		set(v3.data(), v3.size());
	}
	void UniformGLGUBO::set(const std::vector < Vec4 >& v4)
	{
		set(v4.data(), v4.size());
	}
	void UniformGLGUBO::set(const std::vector < Mat3 >& m3)
	{
		set(m3.data(), m3.size());
	}
	void UniformGLGUBO::set(const std::vector < Mat4 >& m4)
	{
		set(m4.data(), m4.size());
	}

	void UniformGLGUBO::set(const std::vector < DVec2 >& v2)
	{
		set(v2.data(), v2.size());
	}
	void UniformGLGUBO::set(const std::vector < DVec3 >& v3)
	{
		set(v3.data(), v3.size());
	}
	void UniformGLGUBO::set(const std::vector < DVec4 >& v4)
	{
		set(v4.data(), v4.size());
	}
	void UniformGLGUBO::set(const std::vector < DMat3 >& m3)
	{
		set(m3.data(), m3.size());
	}
	void UniformGLGUBO::set(const std::vector < DMat4 >& m4)
	{
		set(m4.data(), m4.size());
	}
	bool UniformGLGUBO::is_valid() { return m_context && m_shader && m_buffer; }

	Shader* UniformGLGUBO::get_shader()
	{
		return m_shader;
	}

	UniformGLGUBO::UniformGLGUBO(ContextGL4* context, Shader* shader, unsigned char* buffer, size_t offset)
	: m_context(context)
	, m_shader(shader)
	, m_buffer(buffer)
	, m_offset(offset)
	{
	}

	UniformGLGUBO::UniformGLGUBO()
	{
	}

	UniformGLGUBO::~UniformGLGUBO()
	{
		//none
	}
	  ////////////////////////////////////////////////////////////////////////////////////////////////////
    UniformConstBufferGL4::UniformConstBufferGL4(ContextGL4* context, Shader* shader, GLint id, GLuint  bind)
    :m_context(context)
    ,m_shader(shader)
    ,m_id(id)
	,m_bind(bind)
    {
    }
    UniformConstBufferGL4::UniformConstBufferGL4()
    :m_context(nullptr)
    ,m_shader(nullptr)
    ,m_id(-1)
	,m_bind(0)
    {
    }
    UniformConstBufferGL4::~UniformConstBufferGL4()
    {
        //none
    }
    //bind
    void UniformConstBufferGL4::bind(const ConstBuffer* buffer)
    {
		m_const_buffer = buffer;
		glBindBufferRange(GL_UNIFORM_BUFFER, m_bind, (GLuint)m_context->get_native_CB(m_const_buffer), 0, m_const_buffer->get_size());
	}
    void UniformConstBufferGL4::unbind()
    {
		m_const_buffer = nullptr;
		glBindBufferRange(GL_UNIFORM_BUFFER, m_bind, 0, 0, 0);
    }
    //buffer info
    bool UniformConstBufferGL4::is_valid()
    {
        return m_context && m_shader && m_id != -1;
    }
    Shader* UniformConstBufferGL4::get_shader()
    {
        return m_shader;
    }
    
    ////////////////////
	//     RENDER     //
	////////////////////

    ////////////////////
	// Get Shader Version
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

    static void compute_render_driver_info(ContextGL4* context)
    {
        RenderDriverInfo m_info;
        //type
		context->s_render_driver_info.m_render_driver = DR_OPENGL;
        //Get driver vendor
		context->s_render_driver_info.m_name = (const char*)glGetString(GL_VENDOR);
        //get version
        glGetIntegerv(GL_MAJOR_VERSION, &context->s_render_driver_info.m_major_version);
        glGetIntegerv(GL_MINOR_VERSION, &context->s_render_driver_info.m_minor_version);
        //shader type
		context->s_render_driver_info.m_shader_language = "GLSL";
        //shader version
		context->s_render_driver_info.m_shader_version  = make_test_to_get_shader_version();
    }
        
	bool ContextGL4::init(Video::DeviceResources* resource)
	{
		//disable vsync
		resource->set_vsync(false);
		//init glew
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
		//glFrontFace(GL_CW);
		//Coords like direcX
		//glClipControl(GL_UPPER_LEFT, GL_ZERO_TO_ONE),
		//glClipControl(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE),
#endif
		//clean
		print_errors();
        //get info
        compute_render_driver_info(this);
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

	void ContextGL4::print_info()
	{
		std::string renderer = (const char*)glGetString(GL_RENDERER);
		std::string version = (const char*)glGetString(GL_VERSION);
		std::cout << "Renderer: " << renderer << std::endl;
		std::cout << "OpenGL version supported: " << version << std::endl;
	}

	void ContextGL4::close()
	{
		if (s_vao_attributes) glDeleteVertexArrays(1, &s_vao_attributes);
	}

	RenderDriver ContextGL4::get_render_driver()
	{
		return s_render_driver_info.m_render_driver;
	}
        
    RenderDriverInfo ContextGL4::get_render_driver_info()
    {
        return s_render_driver_info;
    }

	const ClearColorState& ContextGL4::get_clear_color_state()
	{
		return s_render_state.m_clear_color;
	}
        
	void ContextGL4::set_clear_color_state(const ClearColorState& clear_color)
	{
		if (s_render_state.m_clear_color != clear_color)
		{
			s_render_state.m_clear_color = clear_color;
			glClearColor(clear_color.m_color.r, clear_color.m_color.g, clear_color.m_color.b, clear_color.m_color.a);
		}
	}

	void ContextGL4::clear(int type)
	{
        GLbitfield clear_type_gl = 0;
        if(type & CLEAR_COLOR) clear_type_gl |= GL_COLOR_BUFFER_BIT;
        if(type & CLEAR_DEPTH) clear_type_gl |= GL_DEPTH_BUFFER_BIT;
		if(clear_type_gl) glClear(clear_type_gl);
	}

	const DepthBufferState& ContextGL4::get_depth_buffer_state()
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

	void ContextGL4::set_depth_buffer_state(const DepthBufferState& depth)
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

	const CullfaceState& ContextGL4::get_cullface_state()
	{
		return s_render_state.m_cullface;
	}

	void ContextGL4::set_cullface_state(const CullfaceState& cfs)
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

	const ViewportState& ContextGL4::get_viewport_state()
	{
		//return
		return s_render_state.m_viewport;
	}

	void  ContextGL4::set_viewport_state(const ViewportState& vs)
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

	const BlendState& ContextGL4::get_blend_state()
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

	void  ContextGL4::set_blend_state(const BlendState& bs)
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

	const State& ContextGL4::get_render_state()
	{
		return s_render_state;
	}

	void ContextGL4::set_render_state(const State& rs)
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
	ConstBuffer* ContextGL4::create_stream_CB(const unsigned char* data, size_t size)
	{
		auto ptr = new ConstBuffer();
		ptr->gen_buffer();
        ptr->set_size(size);
		//save
		auto last_bind = s_bind_context.m_const_buffer;
		unbind_CB(last_bind);
		//set size
		glBindBuffer(GL_UNIFORM_BUFFER, *ptr);
		glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STREAM_DRAW);
		//bind last
		unbind_CB(last_bind);
		//return
		return ptr;
	}

	VertexBuffer* ContextGL4::create_stream_VBO(const unsigned char* vbo, size_t stride, size_t n)
	{
		auto ptr = new VertexBuffer();
		ptr->gen_buffer();
        ptr->set_size(stride*n);
		//save
		auto last_bind = s_bind_context.m_vertex_buffer;
		unbind_VBO(last_bind);
		//set size
		glBindBuffer(GL_ARRAY_BUFFER, *ptr);
		glBufferData(GL_ARRAY_BUFFER, stride*n, vbo, GL_STREAM_DRAW);
		//bind last
		bind_VBO(last_bind);
		//return
		return ptr;
	}

	IndexBuffer* ContextGL4::create_stream_IBO(const unsigned int* ibo, size_t size)
	{
		auto ptr = new IndexBuffer();
		ptr->gen_buffer();
        ptr->set_size(sizeof(unsigned int)*size);
		//save
		auto last_bind = s_bind_context.m_index_buffer;
		unbind_IBO(last_bind);
		//set size
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ptr);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*size, ibo, GL_STREAM_DRAW);
		//bind last
		bind_IBO(last_bind);
		//return
		return ptr;
	}
		
	ConstBuffer* ContextGL4::create_CB(const unsigned char* data, size_t size)
	{
		auto ptr = new ConstBuffer();
		ptr->gen_buffer();
		ptr->set_size(size);
		//save
		auto last_bind = s_bind_context.m_const_buffer;
		unbind_CB(last_bind);
		//set size
		glBindBuffer(GL_UNIFORM_BUFFER, *ptr);
		glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STATIC_DRAW);
		//bind last
		unbind_CB(last_bind);
		//return
		return ptr;
	}

	VertexBuffer* ContextGL4::create_VBO(const unsigned char* vbo, size_t stride, size_t n)
	{
		auto ptr = new VertexBuffer();
		ptr->gen_buffer();
		ptr->set_size(stride*n);
		//save
		auto last_bind = s_bind_context.m_vertex_buffer;
		unbind_VBO(last_bind);
		//set size
		glBindBuffer(GL_ARRAY_BUFFER, *ptr);
		glBufferData(GL_ARRAY_BUFFER, stride*n, vbo, GL_STATIC_DRAW);
		//bind last
		bind_VBO(last_bind);
		//return
		return ptr;
	}

	IndexBuffer* ContextGL4::create_IBO(const unsigned int* ibo, size_t size)
	{
		auto ptr = new IndexBuffer();
		ptr->gen_buffer();
		ptr->set_size(sizeof(unsigned int)*size);
		//save
		auto last_bind = s_bind_context.m_index_buffer;
		unbind_IBO(last_bind);
		//set size
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ptr);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*size, ibo, GL_STATIC_DRAW);
		//bind last
		bind_IBO(last_bind);
		//return
		return ptr;
	}

	uint64 ContextGL4::get_native_CB(const ConstBuffer* cb) const
	{
		return (uint64)cb->m_id_buffer;
	}

	uint64 ContextGL4::get_native_VBO(const VertexBuffer* vb) const
	{
		return (uint64)vb->m_id_buffer;
	}

	uint64 ContextGL4::get_native_IBO(const IndexBuffer* ib) const
	{
		return (uint64)ib->m_id_buffer;
	}
    
    
    size_t ContextGL4::get_size_CB(const ConstBuffer* cb) const
    {
        return cb->get_size();
    }
    
    size_t ContextGL4::get_size_VBO(const VertexBuffer* vb) const
    {
        return vb->get_size();
    }
    
    size_t ContextGL4::get_size_IBO(const IndexBuffer* ib) const
    {
        return ib->get_size();
    }

	void ContextGL4::update_steam_CB(ConstBuffer* cb, const unsigned char* data, size_t size)
	{
		//get state
		GLint lastbind = 0;
		glGetIntegerv(GL_UNIFORM_BUFFER_BINDING, &lastbind);
		//change
		glBindBuffer(GL_UNIFORM_BUFFER, *cb);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
		//restore
		glBindBuffer(GL_UNIFORM_BUFFER, lastbind);
	}

	void ContextGL4::update_steam_VBO(VertexBuffer* vbo, const unsigned char* data, size_t size)
	{
		//get state
		GLint lastbind = 0;
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &lastbind);
		//change
		glBindBuffer(GL_ARRAY_BUFFER, *vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
		//restore
		glBindBuffer(GL_ARRAY_BUFFER, lastbind);
	}

	void ContextGL4::update_steam_IBO(IndexBuffer* ibo, const unsigned int* data, size_t size) {
		//get state
		GLint lastbind;
		glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &lastbind);
		//change
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ibo);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, size * sizeof(unsigned int), data);
		//restore
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lastbind);
	}
		 
	void ContextGL4::bind_CB(ConstBuffer* cb)
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

	void ContextGL4::bind_VBO(VertexBuffer* vbo)
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

	void ContextGL4::bind_IBO(IndexBuffer* ibo)
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

	void ContextGL4::unbind_CB(ConstBuffer* cb)
	{
		if (cb)
		{
			square_assert(s_bind_context.m_const_buffer == cb);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
			s_bind_context.m_const_buffer = nullptr;
		}
	}

	void ContextGL4::unbind_VBO(VertexBuffer* vbo)
	{
        if(vbo)
        {
            square_assert(s_bind_context.m_vertex_buffer == vbo);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            s_bind_context.m_vertex_buffer = nullptr;
        }
	}

	void ContextGL4::unbind_IBO(IndexBuffer* ibo)
    {
        if(ibo)
        {
            square_assert(s_bind_context.m_index_buffer == ibo);
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

	std::vector<unsigned char> ContextGL4::copy_buffer_CB(const ConstBuffer* cb)
	{
		GLuint sb;
		glGenBuffers(1, &sb);
		glBindBuffer(GL_COPY_READ_BUFFER, (GLuint)*cb);
		glBindBuffer(GL_COPY_WRITE_BUFFER, sb);
		glBufferData(GL_COPY_WRITE_BUFFER, cb->get_size(), nullptr, GL_STATIC_READ);
		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, cb->get_size());

		std::vector<unsigned char> result(cb->get_size());
		glGetBufferSubData(GL_COPY_WRITE_BUFFER, 0, cb->get_size(), result.data());

		glDeleteBuffers(1, &sb);
		return result;
	}

	std::vector<unsigned char> ContextGL4::copy_buffer_VBO(const VertexBuffer* vbo)
	{
		GLuint sb;
		glGenBuffers(1, &sb);
		glBindBuffer(GL_COPY_READ_BUFFER, (GLuint)*vbo);
		glBindBuffer(GL_COPY_WRITE_BUFFER, sb);
		glBufferData(GL_COPY_WRITE_BUFFER, vbo->get_size(), nullptr, GL_STATIC_READ);
		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, vbo->get_size());

		std::vector<unsigned char> result(vbo->get_size());
		glGetBufferSubData(GL_COPY_WRITE_BUFFER, 0, vbo->get_size(), result.data());

		glDeleteBuffers(1, &sb);
		return result;
	}
	
	std::vector<unsigned char> ContextGL4::copy_buffer_IBO(const IndexBuffer* ibo)
	{
		GLuint sb;
		glGenBuffers(1, &sb);
		glBindBuffer(GL_COPY_READ_BUFFER, (GLuint)*ibo);
		glBindBuffer(GL_COPY_WRITE_BUFFER, sb);
		glBufferData(GL_COPY_WRITE_BUFFER, ibo->get_size(), nullptr, GL_STATIC_READ);
		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, ibo->get_size());

		std::vector<unsigned char> result(ibo->get_size());
		glGetBufferSubData(GL_COPY_WRITE_BUFFER, 0, ibo->get_size(), result.data());

		glDeleteBuffers(1, &sb);
		return result;
	}

	unsigned char* ContextGL4::map_CB(ConstBuffer* cb, size_t start, size_t n, MappingType type)
	{
		bind_CB(cb);
		glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
		return (unsigned char*)glMapBufferRange(GL_UNIFORM_BUFFER, start, n, get_mapping_type_map_buff_range(type));
	}

	unsigned char* ContextGL4::map_CB(ConstBuffer* cb, MappingType type)
	{
		bind_CB(cb);
		glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
		return (unsigned char*)glMapBuffer(GL_UNIFORM_BUFFER, get_mapping_type_map_buff_range(type));
	}

	void ContextGL4::unmap_CB(ConstBuffer* cb)
	{
		glUnmapBuffer(GL_UNIFORM_BUFFER);
		unbind_CB(cb);
	}
		
	unsigned char* ContextGL4::map_VBO(VertexBuffer* vbo, size_t start, size_t n, MappingType type)
	{
		bind_VBO(vbo);
		glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
		return (unsigned char*)glMapBufferRange(GL_ARRAY_BUFFER, start, n, get_mapping_type_map_buff_range(type));
	}

	unsigned char* ContextGL4::map_VBO(VertexBuffer* vbo, MappingType type)
	{
		bind_VBO(vbo);
		glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
		return (unsigned char*)glMapBuffer(GL_ARRAY_BUFFER, get_mapping_type_map_buff_range(type));
	}

	void ContextGL4::unmap_VBO(VertexBuffer* vbo)
	{
		glUnmapBuffer(GL_ARRAY_BUFFER);
		unbind_VBO(vbo);
	}

	unsigned int*  ContextGL4::map_IBO(IndexBuffer* ibo, size_t start, size_t n, MappingType type)
	{
		bind_IBO(ibo);
		glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
		return (unsigned int*)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, start * sizeof(unsigned int), n * sizeof(unsigned int), get_mapping_type_map_buff_range(type));
	}

	unsigned int* ContextGL4::map_IBO(IndexBuffer* ibo, MappingType type)
	{
		bind_IBO(ibo);
		glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
		return (unsigned int*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, get_mapping_type_map_buff_range(type));
	}

	void ContextGL4::unmap_IBO(IndexBuffer* ibo)
	{
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
		unbind_IBO(ibo);
	}

	unsigned char* ContextGL4::map_TBO(Texture* tbo, MappingType type)
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

	void ContextGL4::unmap_TBO(Texture* tbo)
	{
		if (tbo)
		{
			square_assert(s_bind_context.m_texture_buffer == tbo);
			//unmap
			glUnmapBuffer(GL_TEXTURE_BUFFER);
			//unbind
			glBindBuffer(GL_TEXTURE_BUFFER, 0);
			//update
			s_bind_context.m_texture_buffer = nullptr;
		}
	}

	void ContextGL4::delete_CB(ConstBuffer*& cb)
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

	void ContextGL4::delete_VBO(VertexBuffer*& vbo)
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

	void ContextGL4::delete_IBO(IndexBuffer*& ibo)
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

	void ContextGL4::draw_arrays(DrawType type, unsigned int n)
	{
		glDrawArrays(get_draw_type(type), 0, n);
	}

	void ContextGL4::draw_arrays(DrawType type, unsigned int start, unsigned int size)
	{
		glDrawArrays(get_draw_type(type), start, size);
	}

	void ContextGL4::draw_elements(DrawType type, unsigned int n)
	{
		glDrawElements(get_draw_type(type), n, GL_UNSIGNED_INT, (void*)NULL);
	}
	void ContextGL4::draw_elements(DrawType type, unsigned int start, unsigned int n)
	{
		glDrawElements(get_draw_type(type), n, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * start));
	}

	/*
		InputLayout
	*/
	static inline const char* attribute_to_string(AttributeType type)
	{
		switch (type)
		{ 
		case Square::Render::ATT_POSITION0: return "POSITION0";
		case Square::Render::ATT_NORMAL0:   return "NORMAL0";
		case Square::Render::ATT_TEXCOORD0: return "TEXCOORD0";
		case Square::Render::ATT_TANGENT0:  return "TANGENT0";
		case Square::Render::ATT_BINORMAL0: return "BINORMAL0";
		case Square::Render::ATT_COLOR0:	return "COLOR0";

		case Square::Render::ATT_POSITION1: return "POSITION1";
		case Square::Render::ATT_NORMAL1:   return "NORMAL1";
		case Square::Render::ATT_TEXCOORD1: return "TEXCOORD1";
		case Square::Render::ATT_TANGENT1:  return "TANGENT1";
		case Square::Render::ATT_BINORMAL1: return "BINORMAL1";
		case Square::Render::ATT_COLOR1:	return "COLOR1";

		case Square::Render::ATT_NORMAL2:   return "NORMAL2";
		case Square::Render::ATT_TEXCOORD2: return "TEXCOORD2";
		case Square::Render::ATT_TANGENT2:  return "TANGENT2";
		case Square::Render::ATT_BINORMAL2: return "BINORMAL2";
		case Square::Render::ATT_COLOR2:	return "COLOR2";

		default: return "";
		}
	}
	static inline const char* hlsl_attribute_to_string(AttributeType type)
	{
		switch (type)
		{
		case Square::Render::ATT_POSITION0: return "in_POSITION0";
		case Square::Render::ATT_NORMAL0:   return "in_NORMAL0";
		case Square::Render::ATT_TEXCOORD0: return "in_TEXCOORD0";
		case Square::Render::ATT_TANGENT0:  return "in_TANGENT0";
		case Square::Render::ATT_BINORMAL0: return "in_BINORMAL0";
		case Square::Render::ATT_COLOR0:	return "in_COLOR0";

		case Square::Render::ATT_POSITION1: return "in_POSITION1";
		case Square::Render::ATT_NORMAL1:   return "in_NORMAL1";
		case Square::Render::ATT_TEXCOORD1: return "in_TEXCOORD1";
		case Square::Render::ATT_TANGENT1:  return "in_TANGENT1";
		case Square::Render::ATT_BINORMAL1: return "in_BINORMAL1";
		case Square::Render::ATT_COLOR1:	return "in_COLOR1";

		case Square::Render::ATT_NORMAL2:   return "in_NORMAL2";
		case Square::Render::ATT_TEXCOORD2: return "in_TEXCOORD2";
		case Square::Render::ATT_TANGENT2:  return "in_TANGENT2";
		case Square::Render::ATT_BINORMAL2: return "in_BINORMAL2";
		case Square::Render::ATT_COLOR2:	return "in_COLOR2";

		default: return "";
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	//InputLayout
	InputLayout::GLLayout InputLayout::build(Shader* shader)
	{
		InputLayout::GLLayout output;
		//add fiels
		for (auto& attr : m_list)
		{
			//get location name
			GLint location = glGetAttribLocation(shader->shader_id(), attribute_to_string(attr.m_attribute));
			//re-try (HLSLcc)
			if (location < 0)
			{
				location = glGetAttribLocation(shader->shader_id(), hlsl_attribute_to_string(attr.m_attribute));
			}
			//add
			if (location > -1)
			{
				output.m_locations.push_back(location);
				output.m_attributes.push(attr);
			}
		}
		return output;
	}

	InputLayout::GLLayout* InputLayout::get(Shader* shader)
	{
		//nullcase
		if (!shader) return nullptr;
		//find
		auto it = m_map_layout.find(shader);
		//ok if find
		if (it != m_map_layout.end()) return &it->second;
		//build
		auto layout = build(shader);
		//if null
		if (!layout.size()) return nullptr;
		//else save
		m_map_layout[shader] = layout;
		//end return
		return &m_map_layout[shader];
	}
	InputLayout* ContextGL4::create_IL(const AttributeList& atl)
	{
		//alloc
		InputLayout* output = new InputLayout();
		//save info
		output->m_list = atl;
		//return output
		return output;
	}

	void ContextGL4::delete_IL(InputLayout*& layout)
	{
		//is bind?
		if (s_bind_context.m_input_layout == layout)
		{
			unbind_IL(layout);
		}
		//delete
		delete  layout;
		layout = nullptr;
	}

	static GLenum gl_vertex_attribute_component(AttributeStripType type)
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

	static bool gl_vertex_attribute_is_integer(AttributeStripType type)
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

	void ContextGL4::bind_IL(InputLayout* layout)
	{
        if(layout && s_bind_context.m_input_layout != layout)
        {
            //bind?
            if(s_bind_context.m_input_layout)
            {
                unbind_IL(s_bind_context.m_input_layout);
            }
			//get 
			InputLayout::GLLayout* gllayout = layout->get(s_bind_context.m_shader);
			//build vertex layout
			if (!gllayout)
			{
				//todo: push error
				return;
			}
			//id location
			GLint location_id = 0;
			//size of vertex
			GLuint vertex_size = layout->m_list.size();
            //bind
			for(auto& attribute : gllayout->m_attributes)
            {
				////////////////////////////////////////////////////////////////////////////////////////
				//get location/data
				GLint location = gllayout->m_locations[location_id++];
				//components
				GLint components = attribute.components();
				//component type
				GLenum component_type = gl_vertex_attribute_component(attribute.m_strip);
				//offset
				auto offset = GL_OFFSET_OF(attribute.m_offset);
				////////////////////////////////////////////////////////////////////////////////////////
				//bind attribute
                glEnableVertexAttribArray(location);
				//types
				if (gl_vertex_attribute_is_integer(attribute.m_strip))
				glVertexAttribIPointer
				(
					location,
					components,
					component_type,
					vertex_size, 
					offset
				);
				else 
				glVertexAttribPointer
				(
					location,
					components,
					component_type,
					GL_FALSE,
					vertex_size, 
					offset
				);
				////////////////////////////////////////////////////////////////////////////////////////
            }
            //save
            s_bind_context.m_input_layout = layout;
        }
	}

	void ContextGL4::unbind_IL(InputLayout* layout)
	{
        if(layout)
        {
            //test
			square_assert(s_bind_context.m_input_layout == layout);
			square_assert(s_bind_context.m_shader);
			//get 
			InputLayout::GLLayout* gllayout = layout->get(s_bind_context.m_shader);
            //unbind
            for (const Attribute& data : gllayout->m_attributes)
            {
                glDisableVertexAttribArray(data.m_attribute);
            }
            //safe
            s_bind_context.m_input_layout = nullptr;
        }
	}
	
	size_t ContextGL4::size_IL(const InputLayout* layout)
	{
		return layout->m_list.size();
	}

	bool ContextGL4::has_a_position_IL(const InputLayout* layout)
	{
		for (auto& in : layout->m_list)
			if (in.is_position_transform())
				return true;
		return false;
	}

	size_t ContextGL4::position_offset_IL(const InputLayout* layout)
	{
		for (auto& in : layout->m_list)
			if (in.is_position_transform())
				return in.m_offset;
		return ~((size_t)0);
	}


	//DEPTH
	float ContextGL4::get_depth(const Vec2& pixel) const
	{
		float depth;
		glReadPixels(GLint(pixel.x), GLint(pixel.y), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
		return depth;
	}

	//RGBA
	Vec4 ContextGL4::get_color(const Vec2& pixel) const
	{
		Vec4 rgba;
		glReadPixels(GLint(pixel.x), GLint(pixel.y), 1, 1, GL_RGBA, GL_FLOAT, &rgba);
		return rgba;
	}
	
	//Textures
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

	Texture* ContextGL4::create_texture
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
		glTexParameteri(ctx_texture->m_type_texture, GL_TEXTURE_WRAP_R, get_texture_edge_type(info.m_edge_r));
		glTexParameteri(ctx_texture->m_type_texture, GL_TEXTURE_MAX_ANISOTROPY_EXT, info.m_anisotropy);
		// Generate mipmaps, by the way
		if (info.m_build_mipmap)
		{
			glTexParameteri(ctx_texture->m_type_texture, GL_TEXTURE_BASE_LEVEL, info.m_mipmap_min);
			glTexParameteri(ctx_texture->m_type_texture, GL_TEXTURE_MAX_LEVEL, info.m_mipmap_max);
			glGenerateMipmap(ctx_texture->m_type_texture);
		}
		//disable texture
		glBindTexture(ctx_texture->m_type_texture, 0);
		//test
		print_errors();
		//return texture
		return ctx_texture;
	}

	Texture* ContextGL4::create_cube_texture
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

	std::vector< unsigned char > ContextGL4::get_texture(Texture* tex, int level)
	{
		// get last bind
		auto last_texture = s_bind_context.m_textures[0];
		//unbind
		unbind_texture(last_texture);
		// bind texture
		bind_texture(tex, 0, 0);
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
		bind_texture(last_texture, 0, 0);
		//return
		return output;
	}

	std::vector< unsigned char > ContextGL4::get_texture(Texture* tex, int cube, int level)
	{
		// get last bind
		auto last_texture = s_bind_context.m_textures[0];
		//unbind
		unbind_texture(last_texture);
		// bind texture
		bind_texture(tex, 0, 0);
		// format
		GLint format, components, width, height;
		// get internal format type of GL texture
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X + cube, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X + cube, 0, GL_TEXTURE_COMPONENTS, &components);
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X + cube, 0, GL_TEXTURE_WIDTH, &width);
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X + cube, 0, GL_TEXTURE_HEIGHT, &height);
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
		glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + cube, 0, image_ret_format, GL_UNSIGNED_BYTE, (GLbyte*)output.data());
		// unbind
		unbind_texture(tex);
		// bind last 
		bind_texture(last_texture, 0, 0);
		//return
		return output;
	}

	void ContextGL4::bind_texture(Texture* ctx_texture, int n, int sempler_id)
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

	void ContextGL4::unbind_texture(Texture* ctx_texture)
	{
        if (ctx_texture)
        {
            //to null
			if(ctx_texture->m_last_bind >= 0)
				s_bind_context.m_textures[ctx_texture->m_last_bind] = nullptr;
            //disable
            ctx_texture->disable_TBO();
        }
	}
        
    void ContextGL4::unbind_texture(int n)
    {
        unbind_texture(s_bind_context.m_textures[n]);
    }
        
	void ContextGL4::delete_texture(Texture*& ctx_texture)
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
	CLASS Shader OpenGL
	*/
	Shader::~Shader()
	{
		//detach and delete all shader
		if (m_shader_id)
		{
			//dealloc global ubo
			destroy_global_ubo();
			//dealloc shaders
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

	void Shader::build_global_UBO()
	{
		//get
		m_global_buffer_ref = (UniformConstBufferGL4*)uniform_const_buffer("_Global");
		//init
		if (m_global_buffer_ref)
		{
			//get size
			GLint global_size{ 0 };
			glGetActiveUniformBlockiv(m_shader_id, m_global_buffer_ref->m_id, GL_UNIFORM_BLOCK_DATA_SIZE, &global_size);
			//alloc CPU
			m_global_buffer_cpu.resize(global_size);
			//alloc GPU
			m_global_buffer_gpu = m_context.create_stream_CB(nullptr, global_size);
		}
	}

	void Shader::destroy_global_ubo()
	{
		m_global_buffer_ref = nullptr;
		m_global_buffer_cpu.clear();
		if (m_global_buffer_gpu) m_context.delete_CB(m_global_buffer_gpu);
	}

	void Shader::bind()
	{
		//update global buffer
		if (m_global_buffer_ref && m_global_buffer_update)
		{
			m_context.update_steam_CB(
				  m_global_buffer_gpu
				, m_global_buffer_cpu.data()
				, m_global_buffer_cpu.size()
			);
			m_global_buffer_update = false;
		}
		//start texture uniform
		m_uniform_ntexture = -1;
		//uniform parogram shaders
		glUseProgram(m_shader_id);
		//bind constant global buffer
		if (m_global_buffer_ref) m_global_buffer_ref->bind(m_global_buffer_gpu);
	}
	void Shader::unbind()
	{
		//disable textures
		while (m_uniform_ntexture >= 0)
		{
			m_context.unbind_texture((int)m_uniform_ntexture);
			--m_uniform_ntexture;
		}
		//unbind constant global buffer
		if (m_global_buffer_ref)
		{
			m_global_buffer_ref->unbind();
		}
		//disable parogram
		glUseProgram(0);
	}


	//get uniform
	Uniform* Shader::uniform(const std::string& uname)
	{
		if (m_global_buffer_ref)
		{
			std::string real_name = "_Global." + uname;
			const char* ufiled_name[1] = { real_name.data() };
			GLuint field_index[1] = { 0 };
			glGetUniformIndices(m_shader_id, 1, ufiled_name, field_index);
			//bad index 
			if(field_index[0] == GL_INVALID_INDEX)
			{
				//legacy
				auto uit = m_uniform_map.find(uname);
				//if find
				if (uit != m_uniform_map.end()) return uit->second.get();
				//else
				GLint uid = glGetUniformLocation(m_shader_id, uname.c_str());
				if (uid < 0) return nullptr;
				//add and return
				return &add_uniform(uname, MakeUnique< UniformGL4 >(m_context.allocator(), &m_context, this, uid));
			}
			//get offset
			GLint  offset[1] = { 0 };
			glGetActiveUniformsiv(m_shader_id, 1, field_index, GL_UNIFORM_OFFSET, offset);
			//build
			UniformGLGUBO* uglobal = new UniformGLGUBO(&m_context, this, m_global_buffer_cpu.data(), offset[0]);
			//add and return
			return &add_uniform(uname, std::move(Unique< Uniform >(uglobal, DefaultDelete(m_context.allocator()))));
		}
		else
		{
			//legacy
			auto uit = m_uniform_map.find(uname);
			//if find
			if (uit != m_uniform_map.end()) return uit->second.get();
			//else
			GLint uid = glGetUniformLocation(m_shader_id, uname.c_str());
			if (uid < 0) return nullptr;
			//add and return
			return &add_uniform(uname, MakeUnique< UniformGL4 >(m_context.allocator(), &m_context, this, uid));
		}
	}

	//get uniform const buffer
	UniformConstBuffer* Shader::uniform_const_buffer(const std::string& uname)
	{
		auto uit = m_uniform_const_buffer_map.find(uname);
		//if find
		if (uit != m_uniform_const_buffer_map.end()) return &uit->second;
		//else
		GLuint uid = glGetUniformBlockIndex(m_shader_id, uname.c_str());
		if (uid == GL_INVALID_INDEX) return nullptr;
		//new index
		GLuint bind = get_new_constat_buffer_bind_index();
		//buind index
		glUniformBlockBinding(m_shader_id, uid, bind);
		//add and return
		return &add_uniform_const_buffer(uname, UniformConstBufferGL4(&m_context, this, uid, bind));

	}

	//new bind index
	unsigned int Shader::get_new_constat_buffer_bind_index()
	{
		return m_bind_cb_index++;
	}

	//update global buffer
	void Shader::global_buffer_bind() { m_global_buffer_update = true; }

	//add error log
	void Shader::push_compiler_error(const ShaderCompileError& error_log)
	{
		m_errors.push_back(std::move(error_log));
	}

	void Shader::push_liker_error(const std::string& error_log)
	{
		m_liker_log += error_log;
		m_liker_log += "\n";
	}

	//help
	Uniform& Shader::add_uniform(const std::string& name, Unique<Uniform>&& u) const
	{
		auto& uref = (m_uniform_map[name] = std::move(u));
		return *uref.get();
	}
	UniformConstBufferGL4& Shader::add_uniform_const_buffer(const std::string& name, const UniformConstBufferGL4& ucb) const
	{
		auto& ucbref = (m_uniform_const_buffer_map[name] = ucb);
		return ucbref;
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
	
	Shader* ContextGL4::create_shader(const std::vector< ShaderSourceInformation >& infos)
	{
		//alloc
		Shader* out_shader = new Shader(*this);
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
		//try to find _Global;
		out_shader->build_global_UBO();
		//return shader
		return out_shader;
	}

	bool ContextGL4::shader_compiled_with_errors(Shader* shader)
	{
		return shader->m_errors.size() != 0;
	}
	
	bool ContextGL4::shader_linked_with_error(Shader* shader)
	{
		return shader->m_liker_log.size() != 0;
	}
	
	std::vector< std::string > ContextGL4::get_shader_compiler_errors(Shader* shader)
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
	
	std::string ContextGL4::get_shader_liker_error(Shader* shader)
	{
		return shader->m_liker_log;
	}

	void ContextGL4::bind_shader(Shader* shader)
	{
		if (s_bind_context.m_shader)
		{
			unbind_shader(s_bind_context.m_shader);
		}
		//save
		s_bind_context.m_shader = shader;
		//uniform parogram shaders
		shader->bind();
	}
	
	void ContextGL4::unbind_shader(Shader* shader)
	{
		if (shader)
		{
			square_assert(s_bind_context.m_shader == shader);
			//unbind input layout
			if (s_bind_context.m_input_layout)
			{
				unbind_IL(s_bind_context.m_input_layout);
			}
			//disable program
			shader->unbind();
			//to null
			s_bind_context.m_shader = nullptr;
		}
	}
    
    void ContextGL4::bind_uniform_CB(const ConstBuffer* buffer, Shader* shader, const std::string& uname)
    {
        //get
        auto* ucb = get_uniform_const_buffer(shader,uname);
        //bind
        if(ucb) ucb->bind(buffer);
    }
    
    void ContextGL4::unbind_uniform_CB(Shader* shader, const std::string& uname)
    {
        //get
        auto* ucb = get_uniform_const_buffer(shader,uname);
        //bind
        if(ucb) ucb->unbind();
    }
    
	void ContextGL4::delete_shader(Shader*& shader)
	{
		if (s_bind_context.m_shader == shader)
		{
			unbind_shader(shader);
		}
		delete shader;
		shader = nullptr;
	}

	Shader* ContextGL4::get_bind_shader() const
	{
		return s_bind_context.m_shader;
	}
    
    Uniform* ContextGL4::get_uniform(Shader* shader,const std::string& uname) const
    {
		return shader->uniform(uname);
    }
    
    UniformConstBuffer* ContextGL4::get_uniform_const_buffer(Square::Render::Shader* shader, const std::string& uname) const
    {
		return shader->uniform_const_buffer(uname);
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

	Target* ContextGL4::create_render_target(const std::vector< TargetField >& textures)
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

	void ContextGL4::enable_render_target(Target* r_target)
	{
        if(r_target && s_bind_context.m_render_target != r_target)
        {
            r_target->enable_FBO();
            s_bind_context.m_render_target = r_target;
        }
	}

	void ContextGL4::disable_render_target(Target* r_target)
	{
        if(r_target)
        {
            square_assert(s_bind_context.m_render_target == r_target);
            r_target->disable_FBO();
            s_bind_context.m_render_target = nullptr;
        }
	}

	void ContextGL4::delete_render_target(Target*& r_target)
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

	inline static GLenum get_copy_render_target_type(TargetType type)
	{
		switch (type)
		{
		case RT_COLOR:		  return GL_COLOR_BUFFER_BIT;
		case RT_DEPTH:		  return GL_DEPTH_BUFFER_BIT;
		case RT_STENCIL:	  return GL_STENCIL_BUFFER_BIT;
		case RT_DEPTH_STENCIL:
		default: 
			square_assert(0); 
			return GL_NONE;  
		break;
		}
	}

	void ContextGL4::copy_target_to_target(
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

	bool ContextGL4::print_errors() const
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
	bool ContextGL4::print_errors(const char* source_file_name, int line) const
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
