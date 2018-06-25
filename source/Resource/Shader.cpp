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
#include "Square/Data/ParserUtils.h"
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
	Shader::Shader(Context& context) : ResourceObject(context)
	{

	}
	Shader::Shader(Context& context, const std::string& path) : ResourceObject(context)
	{
		load(path);
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
	struct ShaderImportLoader
	{
		//lines and files
		size_t  m_n_files{ 0 };
		bool    m_files_as_name{ true };
		bool	m_fail{ false };
		//load
		ShaderImportLoader
		(
			Context& context,
			std::stringstream& source,
			const std::string& source_path,
			Shader::FilepathMap& filepath_map,
			std::string& out,
			bool files_as_name = true,
            const size_t line = 0
		)
		{
			m_n_files = 0;
			m_files_as_name = files_as_name;
			out = load(context, source, source_path, filepath_map, line, 0);
		}

		bool fail() const
		{
			return m_fail;
		}

		bool success() const
		{
			return !m_fail;
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
					//get path
                    switch(*c_effect_line)
                    {
                        case '\"': //include
                            if (Parser::parse_string(line, c_effect_line, sourcefile_name))
                            {
                                sourcefile_path = source_dir.size()
                                                ? source_dir + "/" + sourcefile_name
                                                : sourcefile_name;
                            }
                            else
                            {
								context.add_wrong("shader, include path is invalid: " + source_path);
								m_fail = true;
								return out;
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
									m_fail = true;
									return out;
                                }
                            }
                            else
                            {
								context.add_wrong("shader, include path is invalid: " + source_path);
								m_fail = true;
								return out;
                            }
                        break;
                        default:
                            //error 3
							context.add_wrong("shader, include path is invalid: " + source_path);
							m_fail = true;
							return out;
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
	bool Shader::load(const std::string& path) 
	{
		//buffers
		std::string source;
		//input stream
		std::stringstream source_stream(Filesystem::text_file_read_all(path));
		//process include/import
		ShaderImportLoader preprocess(context(), source_stream, path, m_filepath_map, source);
		//compile
		return  preprocess.success() && compile(source, PreprocessMap());
	}
    
    bool Shader::load(const std::string& path,
                      const std::string& insource,
                      const PreprocessMap& defines,
                      const size_t line)
    {
        //buffers
        std::string source(insource);
        //input stream
        std::stringstream source_stream(source);
        //process include/import
        ShaderImportLoader preprocess(context(), source_stream, path, m_filepath_map, source, true, line);
        //compile
        return  preprocess.success() && compile(source, defines);
    }

	//compile from source
	bool Shader::compile
	(
		const std::string& raw_source,
		const PreprocessMap& defines
	)
	{
		//delete last shader
		if (m_shader) destoy();
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
				int version = std::atoi(std::get<1>(p).c_str());
				if (version > 0) shader_version = version;
				continue;
			}
			//add
			header_string += "#" + std::get<0>(p) + " " + std::get<1>(p) + "\n";
		}
		//end source
		std::string source_header = shader_commond_header + header_string;
		std::string source = source_header + raw_source;
		////////////////////////////////////////////////////////////////////////////////
		//shaders
		Xsc::ShaderInput shader_input_info[Render::ST_N_SHADER];
		Xsc::ShaderOutput shader_output_info[Render::ST_N_SHADER];
		Xsc::Reflection::ReflectionData reflection_data[Render::ST_N_SHADER];

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
            , Xsc::ShaderTarget::ComputeShader
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
		//output
		bool is_hlsl = false;
		if (auto render = context().render())
		{
			is_hlsl = render->get_render_driver_info().m_shader_language == "HLSL";
		}
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
			shader_output_info[type].formatting.lineMarks = true;
			shader_output_info[type].formatting.alwaysBracedScopes = true;
			shader_output_info[type].options.separateShaders = true;
			shader_output_info[type].options.separateSamplers = true;
			shader_output_info[type].nameMangling.inputPrefix = "sq_";
			shader_output_info[type].nameMangling.outputPrefix = "sq_";
			shader_output_info[type].nameMangling.useAlwaysSemantics = true;
			shader_output_info[type].nameMangling.renameBufferFields = true;
			//compile
			{
				//output errors
				CompilerLog logs;
				//pass to compile
				if (Xsc::CompileShader(shader_input_info[type], shader_output_info[type], &logs, &reflection_data[type]))
				{
					//save source
					shader_sources[type] = shader_output[type].str();
					//source split
					extract_version_line(shader_sources[type], shader_headers[type]);
					//if directX get original source code
					if (is_hlsl)
					{
						shader_info.push_back
						(Render::ShaderSourceInformation
						{
							 (Render::ShaderType)type //shader type
							, source_header			  //header
							, raw_source			  //source output ref
							, shader_target_name[type]//source output ref
							, 0						  //line 0
						});
					}
					else
					{
						//add inf
						shader_info.push_back
						(Render::ShaderSourceInformation
						{
							(Render::ShaderType)type   //shader type
							, shader_headers[type]     //header
							, shader_sources[type]     //source output ref
							, 0						   //line 0
						});
					}
				}
				else
				{
					for (auto& entry : reflection_data[type].functions)
					{
						if (entry.ident == shader_target_name[type] || type <= Render::ST_FRAGMENT_SHADER)
						{
							context().add_wrong
							(
								"Error to compile \"" + shader_target_name[type] + "\": \n" + logs.get_errors()
							);
							return false;
						}
					}
					if (!reflection_data[type].functions.size() && type <= Render::ST_FRAGMENT_SHADER)
					{
						context().add_wrong
						(
							"Error to compile \"" + shader_target_name[type] + "\": \n" + logs.get_errors()
						);
						return false;
					}
				}
			}
		}
		////////////////////////////////////////////////////////////////////////////////
		// load shaders from files
		if (auto render = context().render())
		{
			//compile
			m_shader = render->create_shader(shader_info);
			//tests
			if (!m_shader || render->shader_compiled_with_errors(m_shader) || render->shader_linked_with_error(m_shader))
			{
				//fail
				context().add_wrong("Error to shader compile");
				context().add_wrongs(render->get_shader_compiler_errors(m_shader));
				context().add_wrong(render->get_shader_liker_error(m_shader));
				return false;
			}
			//success
			return true;
		}
		return false;
	}


    //get buffer
    Render::Uniform* Shader::uniform(const std::string& name) const
    {
        if (auto render = context().render())
            return render->get_uniform(m_shader, std::string(name));
        return nullptr;
    }
    
    Render::UniformConstBuffer* Shader::constant_buffer(const std::string& name) const
    {
        if (auto render = context().render())
            return render->get_uniform_const_buffer(m_shader, name);
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
		if (auto render = context().render())
			render->bind_shader(m_shader);
	}

	//unbind shader
	void Shader::unbind()
	{
		if (auto render = context().render())
			render->unbind_shader(m_shader);
	}

	//destoy shader
	void Shader::destoy()
	{
		//delete last shader
		if (m_shader)
			if (auto render = context().render())
				render->delete_shader(m_shader);
		m_shader = nullptr;
		//clear info
		m_filepath_map.clear();
		m_uniform_map.clear();
		m_cbuffer_map.clear();
	}
}
}
