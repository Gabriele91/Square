//
//  ParserMaterial.cpp
//  Square
//
//  Created by Gabriele Di Bari on 24/03/25.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#include "Square/Data/ParserUtils.h"
#include "Square/Data/ParserTexture.h"
#include "Square/Core/StringUtilities.h"

namespace Square
{
namespace Parser
{
	//////////////////////////////////////////////////////
	// Context
	std::string Texture::Context::errors_to_string() const
	{
		std::stringstream sbuffer;

		for (auto& error : m_errors)
		{
			sbuffer << "Error:" << error.m_line << ": " << error.m_error << "\n";
		}

		return sbuffer.str();
	}
	//////////////////////////////////////////////////////
	bool Texture::parse(Context& default_context, const std::string& source)
	{
		const char* source_ptr = source.c_str();
		return parse(default_context, source_ptr);
	}

	bool Texture::parse(Context& context, const char*& ptr)
	{
		const char* texture_mag[] =
		{
			"nearest",
			"linear"
		};
		const char* texture_min[] =
		{
			"nearest",
			"nearest_mipmap_nearest",
			"nearest_mipmap_linear",
			"linear",
			"linear_mipmap_nearest",
			"linear_mipmap_linear",
		};
		const char* texture_edge[] =
		{
			"clap",
			"repeat"
		};
		//get type
		while (*ptr != EOF && *ptr != '\0')
		{
			//skip line and comments
			skip_space_and_comments(context.m_line, ptr);
			//mag
			if (cstr_cmp_skip(ptr, "mag_filter") || cstr_cmp_skip(ptr, "mag"))
			{
				//skip line and comments
				skip_space_and_comments(context.m_line, ptr);
				//set mag
				for (unsigned char i = 0; i < SqareArrayLangth(texture_mag); ++i)
				{
					if (cstr_cmp_skip(ptr, texture_mag[i]))
					{
						context.m_attributes.m_mag_filter = (Render::TextureMagFilterType)i;
						break;
					}
				}
			}
			else if (cstr_cmp_skip(ptr, "min_filter") || cstr_cmp_skip(ptr, "min"))
			{
				//skip line and comments
				skip_space_and_comments(context.m_line, ptr);
				//set min
				for (unsigned char i = 0; i < SqareArrayLangth(texture_min); ++i)
				{
					if (cstr_cmp_skip(ptr, texture_min[i]))
					{
						context.m_attributes.m_min_filter = (Render::TextureMinFilterType)i;
						break;
					}
				}
			}
			else if (cstr_cmp_skip(ptr, "wrap_s"))
			{
				//skip line and comments
				skip_space_and_comments(context.m_line, ptr);
				//set wrap
				for (unsigned char i = 0; i < SqareArrayLangth(texture_edge); ++i)
				{
					if (cstr_cmp_skip(ptr, texture_edge[i]))
					{
						context.m_attributes.m_wrap_s = (Render::TextureEdgeType)i;
						break;
					}
				}
			}
			else if (cstr_cmp_skip(ptr, "wrap_t"))
			{
				//skip line and comments
				skip_space_and_comments(context.m_line, ptr);
				//set wrap
				for (unsigned char i = 0; i < SqareArrayLangth(texture_edge); ++i)
				{
					if (cstr_cmp_skip(ptr, texture_edge[i]))
					{
						context.m_attributes.m_wrap_t = (Render::TextureEdgeType)i;
						break;
					}
				}
			}
			else if (cstr_cmp_skip(ptr, "wrap_r"))
			{
				//skip line and comments
				skip_space_and_comments(context.m_line, ptr);
				//set wrap
				for (unsigned char i = 0; i < SqareArrayLangth(texture_edge); ++i)
				{
					if (cstr_cmp_skip(ptr, texture_edge[i]))
					{
						context.m_attributes.m_wrap_r = (Render::TextureEdgeType)i;
						break;
					}
				}
			}
			else if (cstr_cmp_skip(ptr, "wrap"))
			{
				//skip line and comments
				skip_space_and_comments(context.m_line, ptr);
				//set wrap
				for (unsigned char i = 0; i < SqareArrayLangth(texture_edge); ++i)
				{
					if (cstr_cmp_skip(ptr, texture_edge[i]))
					{
						context.m_attributes.m_wrap_s =
						context.m_attributes.m_wrap_t =
						context.m_attributes.m_wrap_r = (Render::TextureEdgeType)i;
						break;
					}
				}
			}
			else if (cstr_cmp_skip(ptr, "mipmap") || cstr_cmp_skip(ptr, "build_mipmap"))
			{
				//skip line and comments
				skip_space_and_comments(context.m_line, ptr);
				//set mipmap
				if (!parse_bool(ptr, context.m_attributes.m_build_mipmap))
				{
					context.m_errors.emplace_back(context.m_line, "Invalid mipmap value");
					return false;
				}
			}
			else if (cstr_cmp_skip(ptr, "image") || cstr_cmp_skip(ptr, "url") || cstr_cmp_skip(ptr, "texture"))
			{
				//skip line and comments
				skip_space_and_comments(context.m_line, ptr);
				//set image name
				if (!parse_string(context.m_line, ptr, context.m_url))
				{
					context.m_errors.emplace_back(context.m_line, "Invalid image/url/texture value");
					return false;
				}
			}
			else
			{
				context.m_errors.emplace_back(context.m_line, "Not found a valid command");
				return false;
			}
			//skip line and comments
			skip_space_and_comments(context.m_line, ptr);
		}
		return true;
	}

}
}