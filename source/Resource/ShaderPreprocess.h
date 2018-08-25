#pragma once
#include "Square/Config.h"
#include "Square/Core/Application.h"
#include "Square/Core/Filesystem.h"
#include "Square/Core/StringUtilities.h"
#include "Square/Core/Context.h"
#include "Square/Data/ParserUtils.h"
#include "Square/Resource/Shader.h"

namespace Square
{
namespace Resource
{
	// Remove Shader comments
	void shader_remove_comments(std::string& source)
	{
		//ptr
		char* csource = &source[0];
		//for all source
		while (*csource)
		{
			if (Parser::is_line_comment(csource))
			{
				while (*csource && *csource != '\n') (*csource++) = ' ';
			}
			else if (Parser::is_start_multy_line_comment(csource))
			{
				//remove /* 
				while (*csource && !Parser::is_end_multy_line_comment(csource)) (*csource++) = ' ';
				//remove */
				if (Parser::is_end_multy_line_comment(csource))
				{
					(*csource++) = ' '; // *
					(*csource++) = ' '; // /
				}
			}
			else ++csource;
		}
	}

	// Extract version
	bool shader_extract_glsl_version_line(std::string& source, std::string& version)
	{
		//lines
		size_t line = 0;
		//line buffer
		std::string source_line;
		//init stram buffer
		std::stringstream source_stream(source);
		//init 
		source.clear();
		version.clear();
		//parsing
		while (std::getline(source_stream, source_line))
		{
			//ptr to line
			const char* c_effect_line = source_line.c_str();
			//jmp space
			Parser::skip_line_space(line, c_effect_line);
			//append to a output
			if (!version.size() && Parser::cstr_cmp_skip(c_effect_line, "#version"))
				version = source_line + "\n";
			else
				source += source_line;
			//add endline
			source += "\n";
		}
		return version.size() != 0;
	}

	// Help parser
	struct ShaderImportLoader
	{
        enum ParserState
        {
            PS_OK              = 1,
            PS_FAIL            = 0,
            PS_OLREADY_INCLUDE = 2
        };
		//load
		ShaderImportLoader
		(
			Context& context,
			const std::string& source,
			const std::string& source_path,
			Shader::FilepathMap& filepath_map,
			std::string& out,
			bool files_as_name = true,
            const size_t line = 0
		)
		{
			m_n_files = 0;
			m_files_as_name = files_as_name;
            m_once_map.clear();
            //source to stream source
            std::stringstream stream_source(source);
            //execute
			m_state = load(context, stream_source, source_path, filepath_map, line, 0, out);
		}

		bool fail() const
		{
			return m_state == PS_FAIL;
		}

		bool success() const
		{
			return m_state != PS_FAIL;
		}

	protected:
        //lines and files
        size_t      m_n_files{ 0 };
        bool        m_files_as_name{ true };
        ParserState m_state{ PS_OK };
        //mapof once
        std::map<std::string,bool> m_once_map;
		//loader
		ParserState load
		(
			Context& context,
			std::stringstream& source,
			const std::string& source_path,
			Shader::FilepathMap& filepath_map,
			size_t line,
			size_t level,
            std::string& out
		)
		{
			//cicle test
			if (level > 16)
			{
				throw std::runtime_error
				(
					"shader, header inclusion depth limit reached: "
					"might be caused by cyclic header inclusion"
				);
			}
			//get base dir
			std::string source_dir = Filesystem::get_directory(source_path);
			//line buffer
			std::string source_line;
			//count files
			size_t  this_file = m_n_files++;
			std::string source_filename = (m_files_as_name ? "\"" + source_path + "\"" : std::to_string(this_file));
			//once? Not reinclude
            auto source_is_once = m_once_map.find(source_filename);
            if(source_is_once != m_once_map.end() && source_is_once->second)
            {
                return PS_OLREADY_INCLUDE;
            }
            //add into the map
			filepath_map[this_file] = source_dir;
			//put line of "all" buffer
            out = "#line " + std::to_string(line + 1) + " " + source_filename  + "\n";
			//start to parse
			while (std::getline(source, source_line))
			{
				//line count
				++line;
				//ptr to line
				const char* c_effect_line = source_line.c_str();
				//jmp space
				Parser::skip_line_space(line, c_effect_line);
				//is pragma?
                if (Parser::cstr_cmp_skip(c_effect_line, "#pragma"))
                {
                    //jmp space
                    Parser::skip_line_space(line, c_effect_line);
                    //get name
                    std::string name;
                    if(Parser::parse_name(c_effect_line, name))
                    {
                        m_once_map[source_filename] = name == "once";
                    }
                    continue;
                }
                //include
                if (Parser::cstr_cmp_skip(c_effect_line, "#include"))
                {
                    //name/path/info
                    std::string sourcefile_name;
                    std::string sourcefile_path;
                    //jmp space
                    Parser::skip_line_space(line, c_effect_line);
					//get path
                    switch(*c_effect_line)
                    {
                        case '\"': //include
                            if (Parser::parse_string(line, c_effect_line, sourcefile_name))
                            {
                                sourcefile_path = Filesystem::join(source_dir, sourcefile_name);
                            }
                            else
                            {
								context.add_wrong("shader, include path is invalid: " + source_path);
								return PS_FAIL;
                            }
                        break;
                        case '<': //import
                            if (Parser::parse_string(line, c_effect_line, sourcefile_name, '<', '>'))
                            {
                                sourcefile_path = context.resource_path<Shader>(sourcefile_name);
                                //test path
                                if(!sourcefile_path.size())
                                {
									context.add_wrong("shader, include path is invalid: " + source_path + ", " + sourcefile_name + " not exists");
									return PS_FAIL;
                                }
                            }
                            else
                            {
								context.add_wrong("shader, include path is invalid: " + source_path);
								return PS_FAIL;
                            }
                        break;
                        default:
                            //error 3
							context.add_wrong("shader, include path is invalid: " + source_path);
							return PS_FAIL;
                        break;
                    };
                    //source
                    std::stringstream source_stream(Filesystem::text_file_read_all(sourcefile_path));
					//ok
                    std::string inc_output;
                    ParserState result = load
                    (
                       context
                     , source_stream
                     , sourcefile_path
                     , filepath_map
                     , 0 //start from 0
                     , level + 1
                     , inc_output
                    );
                    //success
					if(result == PS_OK)
                    {
                        //add file
                        out += inc_output;
                        //lines
                        std::string current_line = "#line " + std::to_string(line + 1) + " " + source_filename  + "\n";
                        out += current_line;
                        //success
                    }
                    //return if is fail 
                    if(result == PS_FAIL) return PS_FAIL;
                }
				else
				{
					//append to a output
					out += source_line;
					out += "\n";
				}
			}
			return PS_OK;
		}
	};

}
}