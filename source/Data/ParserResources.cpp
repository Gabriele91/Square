#include "Square/Core/Filesystem.h"
#include "Square/Data/ParserUtils.h"
#include "Square/Data/ParserResources.h"

namespace Square
{
namespace Parser
{
	//////////////////////////////////////////////////////
	// Context
	std::string Resources::Context::errors_to_string() const
	{
		std::stringstream sbuffer;

		for (auto& error : m_errors)
		{
			sbuffer << "Error:" << error.m_line << ": " << error.m_error << "\n";
		}

		return sbuffer.str();
	}
	//////////////////////////////////////////////////////
	bool Resources::parse(Context& default_context, const std::string& source)
	{
		const char* c_ptr = source.c_str();
		return parse(default_context, c_ptr);
	}

	bool Resources::parse(Context& default_context, const char*& ptr)
	{
		m_context = &default_context;
		//restart
		m_context->m_paths.clear();
		m_context->m_files.clear();
		m_context->m_errors.clear();
		m_context->m_line = 1;
		//get type
		while (*ptr != EOF && *ptr != '\0')
		{
			//skip line and comments
			skip_space_and_comments(m_context->m_line, ptr);
			//parsing a block
			if (cstr_cmp_skip(ptr, "resources"))
			{ 
				if (!parse_resources_block(ptr)) return false; 
			}
			else 
			{ 
				push_error("Not found a valid command"); 
				return false;
			}
			//skip line and comments
			skip_space_and_comments(m_context->m_line, ptr);
		}
		return true;
	}
	//////////////////////////////////////////////////////
	bool Resources::parse_resources_block(const char*& ptr)
	{
		//skip spaces
		skip_space_and_comments(m_context->m_line, ptr);
		//parse table
		if (is_start_table(*ptr))
		{
			//skip '{'
			++ptr;
			//skip spaces
			skip_space_and_comments(m_context->m_line, ptr);
			//read all values
			while (!is_end_table(*ptr) && *ptr != EOF && *ptr != '\0')
			{
				//alloc uniform field
				if (cstr_cmp_skip(ptr, "path"))
				{
					PathField n_path_field;
					//skip spaces
					skip_space_and_comments(m_context->m_line, ptr);
					//parse textures
					if (!parse_string(m_context->m_line, ptr, n_path_field.m_path))
					{
						push_error("Not valid path field");
						return false;
					}
					//test, is recursive
					n_path_field.m_recursive =
						Filesystem::get_filename(n_path_field.m_path) == "**";
					//only dir
					if (n_path_field.m_recursive)
					{
						n_path_field.m_path =
							Filesystem::get_directory(n_path_field.m_path);
					}
					//skip spaces
					skip_space_and_comments(m_context->m_line, ptr);
					//test, name?
					if (cstr_cmp_skip(ptr, "filter"))
					{
						//skip spaces
						skip_space_and_comments(m_context->m_line, ptr);
						//parse textures
						if (!parse_string(m_context->m_line, ptr, n_path_field.m_reg_exp))
						{
							push_error("Not valid filter field");
							return false;
						}
						//enable
						n_path_field.m_filtered = true;
						//skip spaces
						skip_space_and_comments(m_context->m_line, ptr);

					}
					//skip spaces
					skip_space_and_comments(m_context->m_line, ptr);
					//add
					m_context->m_paths.push_back(n_path_field);
				}
				else if (cstr_cmp_skip(ptr, "file"))
				{
					FileField n_file_field;
					//skip spaces
					skip_space_and_comments(m_context->m_line, ptr);
					//parse textures
					if (!parse_string(m_context->m_line, ptr, n_file_field.m_path))
					{
						push_error("Not valid path field");
						return false;
					}
					//skip spaces
					skip_space_and_comments(m_context->m_line, ptr);
					//test, name?
					if (cstr_cmp_skip(ptr, "name"))
					{
						//skip spaces
						skip_space_and_comments(m_context->m_line, ptr);
						//parse textures
						if (!parse_string(m_context->m_line, ptr, n_file_field.m_asset_name))
						{
							push_error("Not valid name field");
							return false;
						}
						//enable
						n_file_field.m_asset_name = true;
						//skip spaces
						skip_space_and_comments(m_context->m_line, ptr);

					}
					//add
					m_context->m_files.push_back(n_file_field);
				}
				else
				{
					push_error("Not valid resource field");
					return false;
				}
				//skip spaces
				skip_space_and_comments(m_context->m_line, ptr);
			}
			//end while
			if (!is_end_table(*ptr))
			{
				push_error("Not found }");
				return false;
			}
			//skip }
			++ptr;
		}
		return true;
	}
	//////////////////////////////////////////////////////
	void Resources::push_error(const std::string& error)
	{
		m_context->m_errors.push_front(ErrorField{ m_context->m_line,error });
	}
}
}