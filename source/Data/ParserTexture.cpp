//
//  ParserTexture.cpp
//  Square
//
//  Created by Gabriele Di Bari on 24/03/25.
//  Copyright � 2018 Gabriele Di Bari. All rights reserved.
//
#include "Square/Data/ParserUtils.h"
#include "Square/Data/ParserTexture.h"
#include "Square/Core/StringUtilities.h"
#include <string_view>
#include <cstring>

namespace Square
{
namespace Parser
{
	// // // // // // // // // // // // // // // // // // // // // // // // // //
	// Keyword constants for the .texture grammar. Centralised so the spelling
	// lives in one place (no scattered string literals).
	namespace keyword
	{
		constexpr std::string_view mag_filter   = "mag_filter";
		constexpr std::string_view mag          = "mag";
		constexpr std::string_view min_filter   = "min_filter";
		constexpr std::string_view min          = "min";
		constexpr std::string_view wrap_s       = "wrap_s";
		constexpr std::string_view wrap_t       = "wrap_t";
		constexpr std::string_view wrap_r       = "wrap_r";
		constexpr std::string_view wrap         = "wrap";
		constexpr std::string_view anisotropic  = "anisotropic";
		constexpr std::string_view mipmap       = "mipmap";
		constexpr std::string_view build_mipmap = "build_mipmap";
		constexpr std::string_view data         = "data";
		constexpr std::string_view image        = "image";
		constexpr std::string_view url          = "url";
		constexpr std::string_view texture      = "texture";
	}
	// // // // // // // // // // // // // // // // // // // // // // // // // //
	// Keyword <-> enum tables and the parsing helpers. Everything is file-static
	// (anonymous namespace): a single source of truth shared by the dispatcher.
	namespace
	{
		using Context = Texture::Context;

		// -- value tables: a value spelling + the matching enum -------------------
		constexpr std::string_view s_texture_mag_names[]
		{
			"nearest_mipmap_nearest",
			"nearest_mipmap_linear",
			"linear_mipmap_nearest",
			"linear_mipmap_linear",
			"linear",
			"nearest",
		};
		constexpr Render::TextureMagFilterType s_texture_mag_map[]
		{
			Render::TextureMagFilterType::TMAG_NEAREST_MIPMAP_NEAREST,
			Render::TextureMagFilterType::TMAG_NEAREST_MIPMAP_LINEAR,
			Render::TextureMagFilterType::TMAG_LINEAR_MIPMAP_NEAREST,
			Render::TextureMagFilterType::TMAG_LINEAR_MIPMAP_LINEAR,
			Render::TextureMagFilterType::TMAG_LINEAR,
			Render::TextureMagFilterType::TMAG_NEAREST,
		};
		constexpr std::string_view s_texture_min_names[]
		{
			"nearest_mipmap_nearest",
			"nearest_mipmap_linear",
			"linear_mipmap_nearest",
			"linear_mipmap_linear",
			"nearest",
			"linear",
		};
		constexpr Render::TextureMinFilterType s_texture_min_map[]
		{
			Render::TextureMinFilterType::TMIN_NEAREST_MIPMAP_NEAREST,
			Render::TextureMinFilterType::TMIN_NEAREST_MIPMAP_LINEAR,
			Render::TextureMinFilterType::TMIN_LINEAR_MIPMAP_NEAREST,
			Render::TextureMinFilterType::TMIN_LINEAR_MIPMAP_LINEAR,
			Render::TextureMinFilterType::TMIN_NEAREST,
			Render::TextureMinFilterType::TMIN_LINEAR,
		};
		constexpr std::string_view s_texture_edge_names[]
		{
			"clamp",
			"repeat",
		};

		// -- value setters: assign the enum picked by index into the attributes ----
		void set_mag   (Context& context, int index) { context.m_attributes.m_mag_filter = s_texture_mag_map[index]; }
		void set_min   (Context& context, int index) { context.m_attributes.m_min_filter = s_texture_min_map[index]; }
		void set_wrap_s(Context& context, int index) { context.m_attributes.m_wrap_s = (Render::TextureEdgeType)index; }
		void set_wrap_t(Context& context, int index) { context.m_attributes.m_wrap_t = (Render::TextureEdgeType)index; }
		void set_wrap_r(Context& context, int index) { context.m_attributes.m_wrap_r = (Render::TextureEdgeType)index; }
		void set_wrap  (Context& context, int index) { context.m_attributes.m_wrap_s =
		                                               context.m_attributes.m_wrap_t =
		                                               context.m_attributes.m_wrap_r = (Render::TextureEdgeType)index; }

		// A value command: keyword + value list (+ size) + a setter taking the index.
		struct ValueCommand
		{
			std::string_view        keyword;
			const std::string_view* values;
			size_t                  size;
			void                  (*set)(Context&, int);
		};
		constexpr ValueCommand s_value_commands[]
		{
			{ keyword::mag_filter, s_texture_mag_names,  SqareArrayLangth(s_texture_mag_names),  set_mag    },
			{ keyword::mag,        s_texture_mag_names,  SqareArrayLangth(s_texture_mag_names),  set_mag    },
			{ keyword::min_filter, s_texture_min_names,  SqareArrayLangth(s_texture_min_names),  set_min    },
			{ keyword::min,        s_texture_min_names,  SqareArrayLangth(s_texture_min_names),  set_min    },
			{ keyword::wrap_s,     s_texture_edge_names, SqareArrayLangth(s_texture_edge_names), set_wrap_s },
			{ keyword::wrap_t,     s_texture_edge_names, SqareArrayLangth(s_texture_edge_names), set_wrap_t },
			{ keyword::wrap_r,     s_texture_edge_names, SqareArrayLangth(s_texture_edge_names), set_wrap_r },
			{ keyword::wrap,       s_texture_edge_names, SqareArrayLangth(s_texture_edge_names), set_wrap   },
		};

		// -- basic commands: keyword + a handler that reads its value and assigns --
		bool handle_anisotropic(Context& context, const char*& cursor, const char* /*buffer_end*/)
		{
			skip_space_and_comments(context.m_line, cursor);
			if (!parse_int(cursor, context.m_attributes.m_anisotropic))
			{
				context.m_errors.emplace_back(context.m_line, "Invalid anisotropic value");
				return false;
			}
			return true;
		}
		bool handle_mipmap(Context& context, const char*& cursor, const char* /*buffer_end*/)
		{
			skip_space_and_comments(context.m_line, cursor);
			if (!parse_bool(cursor, context.m_attributes.m_build_mipmap))
			{
				context.m_errors.emplace_back(context.m_line, "Invalid mipmap value");
				return false;
			}
			return true;
		}
		bool handle_data(Context& context, const char*& cursor, const char* buffer_end)
		{
			//copy the rest of the buffer as raw bytes and end the parsing
			const unsigned char* data_start = reinterpret_cast<const unsigned char*>(cursor);
			const unsigned char* data_end   = reinterpret_cast<const unsigned char*>(buffer_end);
			context.m_image = std::vector<unsigned char>{ data_start, data_end };
			cursor = buffer_end;
			return true;
		}
		bool handle_image(Context& context, const char*& cursor, const char* /*buffer_end*/)
		{
			skip_space_and_comments(context.m_line, cursor);
			std::string url;
			if (!parse_string(context.m_line, cursor, url))
			{
				context.m_errors.emplace_back(context.m_line, "Invalid image/url/texture value");
				return false;
			}
			context.m_image = url;
			return true;
		}

		// A command: keyword + a handler (false on parse error, error already pushed).
		struct Command
		{
			std::string_view keyword;
			bool           (*handle)(Context&, const char*&, const char* buffer_end);
		};
		constexpr Command s_basic_commands[]
		{
			{ keyword::anisotropic,  handle_anisotropic },
			{ keyword::mipmap,       handle_mipmap      },
			{ keyword::build_mipmap, handle_mipmap      },
			{ keyword::data,         handle_data        },
			{ keyword::image,        handle_image       },
			{ keyword::url,          handle_image       },
			{ keyword::texture,      handle_image       },
		};

		// -- sub-parsers: each returns NotFound / Handled / Error ------------------
		enum class CommandResult { NotFound, Handled, Error };

		CommandResult parse_value_commands(Context& context, const char*& cursor, const char* /*buffer_end*/)
		{
			for (const auto& command : s_value_commands)
			{
				if (!cstr_cmp_skip(cursor, command.keyword)) continue;
				//skip line and comments
				skip_space_and_comments(context.m_line, cursor);
				//match the value and set it
				for (size_t index = 0; index < command.size; ++index)
				{
					if (cstr_cmp_skip(cursor, command.values[index]))
					{
						command.set(context, (int)index);
						return CommandResult::Handled;
					}
				}
				//keyword matched but no valid value: report it instead of silently ignoring
				context.m_errors.emplace_back(context.m_line, "Invalid value for '" + std::string(command.keyword) + "'");
				return CommandResult::Error;
			}
			return CommandResult::NotFound;
		}
		CommandResult parse_basic_commands(Context& context, const char*& cursor, const char* buffer_end)
		{
			for (const auto& command : s_basic_commands)
			{
				if (!cstr_cmp_skip(cursor, command.keyword)) continue;
				return command.handle(context, cursor, buffer_end) ? CommandResult::Handled : CommandResult::Error;
			}
			return CommandResult::NotFound;
		}

		// The super-parser: an ordered list of group parsers; the first that
		// recognizes the keyword wins (NotFound -> try the next one).
		using Handler = CommandResult (*)(Context& context, const char*& cursor, const char* buffer_end);
		constexpr Handler s_handlers[]
		{
			parse_value_commands,
			parse_basic_commands,
		};
	}
	// // // // // // // // // // // // // // // // // // // // // // // // // //
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
	// // // // // // // // // // // // // // // // // // // // // // // // // //
	bool Texture::parse(Context& default_context, const std::string& source)
	{
		const char* source_ptr = source.c_str();
		return parse(default_context, source_ptr, source.size());
	}

	bool Texture::parse(Context& context, const char* ptr, size_t size)
	{
		//set start line
		context.m_line = 1;
		//end of the memory
		const char* buffer_end = ptr + size;
		//get type
		while (*ptr != '\0' && ptr < buffer_end)
		{
			//skip line and comments
			skip_space_and_comments(context.m_line, ptr);
			//nothing left but trailing space/comments
			if (*ptr == '\0' || ptr >= buffer_end) break;
			//dispatch to the first handler that recognizes the keyword
			bool handled = false;
			for (const auto& handler : s_handlers)
			{
				const CommandResult result = handler(context, ptr, buffer_end);
				if (result == CommandResult::Error)   return false; // error already in m_errors
				if (result == CommandResult::Handled) { handled = true; break; }
				//NotFound -> try the next handler
			}
			//unknown command
			if (!handled)
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
