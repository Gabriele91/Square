//
//  Square
//
//  Created by Gabriele on 29/06/16.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#pragma once
//Engine include
#include "Square/Config.h"
//Std include
#include <sstream>
//OS Include
#ifdef __APPLE__
    #include <OpenGL/gl3.h>
    #include <OpenGL/glext.h>
#else
    #ifdef _WIN32
        #include <windows.h>
    #endif
	#include <GL/glew.h>
    #include <GL/gl.h>
#endif

namespace Square
{
namespace Debug
{
	static inline const char* get_open_gl_error(GLuint error)
	{
		struct token_string
		{
			GLuint m_token;
			const char* m_string;
		};
		static const struct token_string gl_errors[] =
		{
			{ GL_NO_ERROR, "no error" },
			{ GL_INVALID_ENUM, "invalid enumerant" },
			{ GL_INVALID_VALUE, "invalid value" },
			{ GL_INVALID_OPERATION, "invalid operation" },
#if defined __gl_h_
			{ GL_STACK_OVERFLOW, "stack overflow" },
			{ GL_STACK_UNDERFLOW, "stack underflow" },
			{ GL_TABLE_TOO_LARGE, "table too large" },
#endif
			{ GL_OUT_OF_MEMORY, "out of memory" },
			{ GL_INVALID_FRAMEBUFFER_OPERATION, "invalid framebuffer operation" },
			{ ~static_cast<GLuint>(0), NULL } /* end of list indicator */
		};

		for (int i = 0; gl_errors[i].m_string; i++)
		{
			if (gl_errors[i].m_token == error)
				return (const char *)gl_errors[i].m_string;
		}
		return nullptr;
	}

	inline std::string gl_errors_to_string(const std::string& filename, size_t line)
	{
		std::stringstream output;
		GLenum gl_err = GL_NO_ERROR;
		bool print_file = false;
		GLuint finite = 255; // (watchdog count)
		while ((gl_err = glGetError()) != GL_NO_ERROR && --finite)
		{
			if (!print_file)
			{
				output << "At file: " << filename << " (" << line << ")"<< ":" << std::endl;
			}
			const char* gl_err_str = Debug::get_open_gl_error(gl_err); \
				output << "OpenGL error: " << gl_err << " : " << (gl_err_str ? gl_err_str : "unknow") << std::endl; \
		}
		return output.str();
	}
}
}
#ifdef _DEBUG
	#define debug_gl_errors_to_string_args(source,line) ::Square::Debug::gl_errors_to_string(source,line)
	#define debug_gl_errors_to_string() ::Square::Debug::gl_errors_to_string(__FILE__,__LINE__)
#else
	#define debug_gl_errors_to_string_args(source,line) ::std::string()
	#define debug_gl_errors_to_string() ::std::string()
#endif
