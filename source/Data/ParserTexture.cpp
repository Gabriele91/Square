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
		return parse(default_context, source_ptr, source.size());
	}

	bool Texture::parse(Context& context, const char* ptr, size_t size)
	{
		const char* texture_mag[] =
		{
			"nearest_mipmap_nearest", // noramly converted in nearest
			"nearest_mipmap_linear",  // noramly converted in nearest
			"linear_mipmap_nearest",  // noramly converted in linear
			"linear_mipmap_linear",   // noramly converted in linear
			"linear",
			"nearest",
		};
		const Render::TextureMagFilterType texture_mag_map[]
		{
			Render::TextureMagFilterType::TMAG_NEAREST_MIPMAP_NEAREST,
			Render::TextureMagFilterType::TMAG_NEAREST_MIPMAP_LINEAR,
			Render::TextureMagFilterType::TMAG_LINEAR_MIPMAP_NEAREST,
			Render::TextureMagFilterType::TMAG_LINEAR_MIPMAP_LINEAR,
			Render::TextureMagFilterType::TMAG_NEAREST,
			Render::TextureMagFilterType::TMAG_LINEAR,
		};
		const char* texture_min[] =
		{
			"nearest_mipmap_nearest",
			"nearest_mipmap_linear",
			"linear_mipmap_nearest",
			"linear_mipmap_linear",
			"nearest",
			"linear",
		};
		const Render::TextureMinFilterType texture_min_map[]
		{
			Render::TextureMinFilterType::TMIN_NEAREST_MIPMAP_NEAREST,
			Render::TextureMinFilterType::TMIN_NEAREST_MIPMAP_LINEAR,
			Render::TextureMinFilterType::TMIN_LINEAR_MIPMAP_NEAREST,
			Render::TextureMinFilterType::TMIN_LINEAR_MIPMAP_LINEAR,
			Render::TextureMinFilterType::TMIN_NEAREST,
			Render::TextureMinFilterType::TMIN_LINEAR,
		};
		const char* texture_edge[] =
		{
			"clap",
			"repeat"
		};
		//set start line
		context.m_line = 1;
		//end of the memory
		const char* buffer_end = ptr + size;
		//get type
		while (*ptr != EOF && *ptr != '\0' && ptr < buffer_end)
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
						context.m_attributes.m_mag_filter = texture_mag_map[i];
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
						context.m_attributes.m_min_filter = texture_min_map[i];
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
			else if (cstr_cmp_skip(ptr, "anisotropic"))
			{
				//skip line and comments
				skip_space_and_comments(context.m_line, ptr);
				//read int
				if (!parse_int(ptr, context.m_attributes.m_anisotropic))
				{
					context.m_errors.emplace_back(context.m_line, "Invalid anisotropic value");
					return false;
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
				// get url
				std::string url;
				// set image name
				if (!parse_string(context.m_line, ptr, url))
				{
					context.m_errors.emplace_back(context.m_line, "Invalid image/url/texture value");
					return false;
				}
				else
				{
					context.m_image = url;
				}
			}
			else if (cstr_cmp_skip(ptr, "data"))
			{
				//copy buffer
				const unsigned char* data_start = reinterpret_cast<const unsigned char*>(ptr);
				const unsigned char* data_end = reinterpret_cast<const unsigned char*>(buffer_end);
				context.m_image = std::vector<unsigned char>{ data_start, data_end };
				return true;
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