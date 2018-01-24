//  Square
//
//  Created by Gabriele on 15/08/16.
//  Copyright � 2016 Gabriele. All rights reserved.
//
#include "Square/Config.h"
#include "Square/Core/Application.h"
#include "Square/Core/Filesystem.h"
#include "Square/Core/Context.h"
#include "Square/Core/Attribute.h"
#include "Square/Data/ParseUtils.h"
#include "Square/Resource/Shader.h"
#include "Square/Core/ClassObjectRegistration.h"
//compiler Xsc
#include <iostream>
#include <Xsc/Xsc.h>
#include <fstream>
#include <sstream>
#include <list>


namespace Square
{
namespace Resource
{
    //////////////////////////////////////////////////////////////
    //Add element to objects
    SQUARE_CLASS_OBJECT_REGISTRATION(Shader);
    //Registration in context
    void Shader::object_registration(Context& ctx)
    {
        //factory
        ctx.add_resource<Shader>({ ".glsl", ".hlsl" });
		//attributes
        #if 0
		ctx.add_attributes<Shader>(attribute_function<Shader, void*>
		("constant_buffer"
		, (void*)(nullptr)
		, [](const Shader* shader) -> void* { return (void*)shader->constant_buffer(name); }
		, [](Shader* shader, const void*&) { /*none*/ }
		));
        #endif
        //end
    }

	//Contructor
	Shader::Shader()
	{

	}
	Shader::Shader(const std::string& path)
	{
		load(path);
	}
	Shader::Shader(Context& context, const std::string& path)
	{
		load(context, path);
	}
	
	//Destructor
	Shader::~Shader()
	{
		destoy();
	}

	// Help compiler
	class CompilerLog : public Xsc::Log
	{
	public:
		//! Submits the specified report.
		virtual void SubmitReport(const Xsc::Report& report)
		{
			switch (report.Type())
			{
			case Xsc::ReportTypes::Info:
				m_info_list.push_back(report);
				break;
			case Xsc::ReportTypes::Warning:
				m_warning_list.push_back(report);
				break;
			case Xsc::ReportTypes::Error:
				m_error_list.push_back(report);
				if(!m_entry_point_not_found) 
					m_entry_point_not_found =
					   report.Message().find("entry") != std::string::npos
					&& report.Message().find("point") != std::string::npos
					&& report.Message().find("not")   != std::string::npos
					&& report.Message().find("found") != std::string::npos
					;
				break;
			}
		}

		//errors to string
		std::string get_errors() const
		{
			std::stringstream ss;
			for (auto error : m_error_list)
			{
				ss << error.Message();
				ss << std::endl;
			}
			return ss.str();
		}

		//warning to string
		std::string get_warning() const
		{
			std::stringstream ss;
			for (auto warning : m_warning_list)
			{
				ss << warning.Message();
				ss << std::endl;
			}
			return ss.str();
		}

		//warning to string
		std::string get_info() const
		{
			std::stringstream ss;
			for (auto info : m_info_list)
			{
				ss << info.Message();
				ss << std::endl;
			}
			return ss.str();
		}

		bool entry_point_not_found()
		{
			return m_entry_point_not_found;
		}

	protected:
		//error info
		std::list<Xsc::Report> m_error_list;
		std::list<Xsc::Report> m_warning_list;
		std::list<Xsc::Report> m_info_list;
		bool m_entry_point_not_found{ false };
	};
	
	// Help parser
	struct ShaderLoader
	{
		//lines and files
		size_t  m_n_files{ 0 };
		bool    m_files_as_name{ true };
		//load
		ShaderLoader
		(
			Context& context,
			std::stringstream& source,
			const std::string& source_path,
			Shader::FilepathMap& filepath_map,
			std::string& out,
			bool files_as_name = true
		)
		{
			m_n_files = 0;
			m_files_as_name = files_as_name;
			out = load(context, source, source_path, filepath_map, 0, 0);
		}

	protected:
		//loader
		std::string load
		(
			Context& context,
			std::stringstream& source,
			const std::string& source_path,
			Shader::FilepathMap& filepath_map,
			size_t line,
			size_t level
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
			//add into the map
			filepath_map[this_file] = source_dir;
			//put line of "all" buffer
			std::string out = "#line " + std::to_string(line + 1) + " " + source_filename  + "\n";
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
				if (Parser::cstr_cmp_skip(c_effect_line, "#include"))
                {
                    //name/path/info
                    std::string sourcefile_name;
                    std::string sourcefile_path;
                    //jmp space
                    Parser::skip_line_space(line, c_effect_line);
                    //type, include or import
                    enum include_state
                    {
                        IS_INCLUDE,
                        IS_IMPORT,
                        IS_UNKWNON
                    };
                    include_state parser_state =
                    (*c_effect_line == '\"'
                     ? IS_INCLUDE
                     : (*c_effect_line == '<'
                        ? IS_IMPORT
                        : IS_UNKWNON
                    ));
					//get path
                    switch(parser_state)
                    {
                        case IS_INCLUDE:
                            if (Parser::parse_string(line, c_effect_line, sourcefile_name))
                            {
                                sourcefile_path = source_dir.size()
                                                ? source_dir + "/" + sourcefile_name
                                                : sourcefile_name;
                            }
                            else
                            {
                                throw std::runtime_error ( "shader, include path is invalid: " + source_path );
                            }
                        break;
                        case IS_IMPORT:
                            if (Parser::parse_string(line, c_effect_line, sourcefile_name, '<', '>'))
                            {
                                sourcefile_path = context.resource_path<Shader>(sourcefile_name);
                                //test path
                                if(!sourcefile_path.size())
                                {
                                    throw std::runtime_error
                                    (
                                         "shader, include path is invalid: " + source_path + ", " + sourcefile_name + " not exists"
                                    );
                                }
                            }
                            else
                            {
                                throw std::runtime_error ( "shader, include path is invalid: " + source_path );
                            }
                        break;
                        default:
                            //error 3
                            throw std::runtime_error ( "shader, include path is invalid: " + source_path );
                        break;
                    };
                    //source
                    std::stringstream source_stream(Filesystem::text_file_read_all(sourcefile_path));
					//ok
					out = load
					(
						  context
						, source_stream
						, sourcefile_path
						, filepath_map
						, line
						, level + 1
					);
					//lines
					std::string current_line = "#line " + std::to_string(line + 1) + " " + source_filename  + "\n";
					out += current_line;
				}
				else
				{
					//append to a output
					out += source_line;
					out += "\n";
				}
			}
			return out;
		}
	};

	// Extract version
	bool extract_version_line(std::string& source, std::string& version)
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

	//load shader
	bool Shader::load(Context& context, const std::string& path) 
	{
		//id file
		size_t  this_file = 0;
		//buffers
		std::string source;
		//process
		try
		{
			//input stream
			std::stringstream source_stream(Filesystem::text_file_read_all(path));
			//load
			ShaderLoader(context, source_stream, path, m_filepath_map, source);
			//ok
			return compile(source, PreprocessMap());
		}
		catch (std::exception e)
		{
			std::cerr << e.what() << std::endl;
		}
		//fail
		return false;
	}

	bool Shader::load(const std::string& path)
	{
		return load(*Application::context(), path);
	}

	//compile from source
	bool Shader::compile
	(
		const std::string& raw_source,
		const PreprocessMap& defines
	)
	{
		//delete last shader
		if (m_shader) Render::delete_shader(m_shader);
        //commondo header
        const static std::string shader_commond_header
        (
        "#define IVec2 int2\n"
        "#define IVec3 int3\n"
        "#define IVec4 int4\n"
        "#define IMat3 int3x3\n"
        "#define IMat4 int4x4\n"
        "#define Vec2 float2\n"
        "#define Vec3 float3\n"
        "#define Vec4 float4\n"
        "#define Mat3 float3x3\n"
        "#define Mat4 float4x4\n"
        "#define DVec2 double2\n"
        "#define DVec3 double3\n"
        "#define DVec4 double4\n"
        "#define DMat3 double3x3\n"
        "#define DMat4 double4x4\n"
        );
		//int shader version
		int shader_version = 410;
		//list define
		std::string header_string;
		//defines
		for (auto& p : defines)
		{
			//get version
			if (std::get<0>(p) == "version")
			{
				shader_version = std::atoi(std::get<1>(p).c_str());
				continue;
			}
			//add
			header_string += "#" + std::get<0>(p) + " " + std::get<1>(p) + "\n";
		}
		//end source
		std::string source = shader_commond_header + header_string + raw_source;
		////////////////////////////////////////////////////////////////////////////////
		//shaders
		Xsc::ShaderInput shader_input_info[Render::ST_N_SHADER];
		Xsc::ShaderOutput shader_output_info[Render::ST_N_SHADER];
		std::stringstream  shader_output[Render::ST_N_SHADER];
		std::string shader_headers[Render::ST_N_SHADER];
		std::string shader_sources[Render::ST_N_SHADER];
		std::vector< Render::ShaderSourceInformation > shader_info;
		//Square -> Xsc map
		Xsc::ShaderTarget shader_target_map[Render::ST_N_SHADER]
		{
			  Xsc::ShaderTarget::VertexShader
			, Xsc::ShaderTarget::FragmentShader
			, Xsc::ShaderTarget::GeometryShader
			, Xsc::ShaderTarget::TessellationControlShader
			, Xsc::ShaderTarget::TessellationEvaluationShader
			, Xsc::ShaderTarget::Undefined //compute shader
		};
		std::string shader_target_name[Render::ST_N_SHADER] 
		{
			  "vertex"
			, "fragment"
			, "geometry"
			, "tass_control"
			, "tass_eval"
			, "compute"
		};

		//init all inputs
		for (unsigned short type = 0; type != Render::ST_N_SHADER; ++type)
		{	
			//supported?
			if (shader_target_map[type] == Xsc::ShaderTarget::Undefined) continue;
			//input
			shader_input_info[type].sourceCode    = std::make_shared<std::stringstream>(source);
			shader_input_info[type].shaderVersion = Xsc::InputShaderVersion::HLSL5;
			shader_input_info[type].shaderTarget  = shader_target_map[type];
			shader_input_info[type].entryPoint    = shader_target_name[type];
			//output
			shader_output_info[type].sourceCode    = &shader_output[type];
			shader_output_info[type].shaderVersion = (Xsc::OutputShaderVersion)shader_version;
			//compile
			{
				//output errors
				CompilerLog logs;
				//pass to compile
				if (Xsc::CompileShader(shader_input_info[type], shader_output_info[type], &logs))
				{
					//save source
					shader_sources[type] = shader_output[type].str();
					//source split
					extract_version_line(shader_sources[type], shader_headers[type]);
					//add inf
					shader_info.push_back
					(Render::ShaderSourceInformation
					{
						 (Render::ShaderType)type   //shader type
					   , shader_headers[type]       //header
					   , shader_sources[type]       //source output ref
					   , 0						    //line 0
					});
				}
				else
				{
					if (!logs.entry_point_not_found())
					{
						throw std::runtime_error
						(
							 "Error to compile \"" + shader_target_name[type] + "\": \n"
							+ logs.get_errors()
						);
					}
				}
			}
		}
		////////////////////////////////////////////////////////////////////////////////
		// load shaders from files
		//compile
		m_shader = Render::create_shader(shader_info);
		//return success
		return m_shader
			&& !Render::shader_compiled_with_errors(m_shader)
			&& !Render::shader_linked_with_error(m_shader);
	}

	//get buffer
	Render::Uniform* Shader::uniform(const std::string& name)
	{
		return Render::get_uniform(m_shader, std::string(name));
	}

	Render::ConstBuffer* Shader::constant_buffer(const std::string& name) const
	{
		return nullptr;
	}

	//program
	Render::Shader* Shader::base_shader() const
	{
		return m_shader;
	}

	//bind shader
	void Shader::bind()
	{
		Render::bind_shader(m_shader);
	}

	//unbind shader
	void Shader::unbind()
	{
		Render::unbind_shader(m_shader);
	}

	//destoy shader
	void Shader::destoy()
	{
		//delete last shader
		if (m_shader) Render::delete_shader(m_shader);
		m_shader = nullptr;
	}
}
}
