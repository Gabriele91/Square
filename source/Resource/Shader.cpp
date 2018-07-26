//  Square
//
//  Created by Gabriele on 15/08/16.
//  Copyright � 2016 Gabriele. All rights reserved.
//
#include "Square/Config.h"
#include "Square/Core/Application.h"
#include "Square/Core/Filesystem.h"
#include "Square/Core/StringUtilities.h"
#include "Square/Core/Context.h"
#include "Square/Core/Attribute.h"
#include "Square/Data/ParserUtils.h"
#include "Square/Resource/Shader.h"
#include "Square/Core/ClassObjectRegistration.h"
//compiler HLSL2ALL
#include <HLSL2ALL/HLSL2ALL.h>

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
					out += load
					(
						  context
						, source_stream
						, sourcefile_path
						, filepath_map
						, 0 //start from 0
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
        (R"HLSL(
		#define IVec2 int2
		#define IVec3 int3
		#define IVec4 int4
		#define IMat3 int3x3
		#define IMat4 int4x4
		#define Vec2 float2
		#define Vec3 float3
		#define Vec4 float4
		#define Mat3 float3x3
		#define Mat4 float4x4
		#define DVec2 double2
		#define DVec3 double3
		#define DVec4 double4
		#define DMat3 double3x3
		#define DMat4 double4x4
		#define Sampler2D(name) SamplerState  name; Texture2D name ## _texture_2D;
		#define Sampler3D(name) SamplerState  name; Texture3D name ## _texture_3D;
		#define SamplerCube(name) SamplerState  name; TextureCube name ## _texture_CUBE;
        #define texture2D(name,pos) name ## _texture_2D.Sample(name,pos)
        #define texture3D(name,pos) name ## _texture_3D.Sample(name,pos)
        #define textureCube(name,pos) name ## _texture_CUBE.Sample(name,pos)
		)HLSL");
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
		//output
		bool is_hlsl = false;
		if (auto render = context().render())
		{
			is_hlsl = render->get_render_driver_info().m_shader_language == "HLSL";
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////
		//inputs
		std::string shader_target_name[Render::ST_N_SHADER]
		{
			  "vertex"
            , "fragment"
			, "geometry"
            , "tass_control"
            , "tass_eval"
			, "compute"
		};
		//save types
		HLSL2ALL::TextureSamplerList shader_glsl_tslist;
		HLSL2ALL::TypeSpirvShaderList shader_spirv_outputs;
		HLSL2ALL::ErrorSpirvShaderList shader_spirv_errors;
		HLSL2ALL::InfoSpirvShaderList shader_spirv_info
		{
			{ HLSL2ALL::ST_VERTEX_SHADER, shader_target_name[Render::ST_VERTEX_SHADER] },
			{ HLSL2ALL::ST_TASSELLATION_CONTROL_SHADER, shader_target_name[Render::ST_TASSELLATION_CONTROL_SHADER] },
			{ HLSL2ALL::ST_TASSELLATION_EVALUATION_SHADER, shader_target_name[Render::ST_TASSELLATION_EVALUATION_SHADER] },
            { HLSL2ALL::ST_GEOMETRY_SHADER, shader_target_name[Render::ST_GEOMETRY_SHADER] },
            { HLSL2ALL::ST_FRAGMENT_SHADER, shader_target_name[Render::ST_FRAGMENT_SHADER] },
			{ HLSL2ALL::ST_COMPUTE_SHADER, shader_target_name[Render::ST_COMPUTE_SHADER] },
		};
		//info
		HLSL2ALL::TargetShaderInfo spirv_info;
		spirv_info.m_client_version = 450;
		spirv_info.m_desktop = true;
		spirv_info.m_reverse_mul = true;
		spirv_info.m_vulkan = false;
		spirv_info.m_upgrade_texture_to_samples = false;
		//build
		if (!HLSL2ALL::hlsl_to_spirv(
			  source
			, resource_name()
			, shader_spirv_info
			, shader_spirv_outputs
			, shader_spirv_errors
			, spirv_info
		))
		{
			context().add_wrong("Error to shader compile");
			context().add_wrongs(shader_spirv_errors);
			return false;
		}		
		////////////////////////////////////////////////////////////////////////////////////////////////////
		//init all inputs
		std::vector< Render::ShaderSourceInformation > shader_info;
		//compile
		std::string shader_headers[Render::ST_N_SHADER];
		std::string shader_sources[Render::ST_N_SHADER];
        //config
        HLSL2ALL::GLSLConfig glsl_config;
		glsl_config.m_version = 410;
		glsl_config.m_es = false;
		glsl_config.m_vulkan_semantics = false;
		glsl_config.m_rename_position_in_position0 = true;
		glsl_config.m_fixup_clipspace = true;
		glsl_config.m_flip_vert_y = false;
		glsl_config.m_enable_420pack_extension = false;
		//to HLSL/GLSL
		for (const HLSL2ALL::TypeSpirvShader& ssoutput : shader_spirv_outputs)
		{
			//unpack
            int   type = ssoutput.m_type;
            auto& shader_spirv_out = ssoutput.m_shader;
			//convert 
			if (is_hlsl)
			{
				//header
				shader_headers[type] = "#pragma pack_matrix( row_major )\n" + source_header;
				//add inf
				shader_info.push_back
				(Render::ShaderSourceInformation
				{
					 (Render::ShaderType)type //shader type
					, shader_headers[type]	  //header
					, raw_source			  //source output ref
					, shader_target_name[type]//source output ref
					, 0						  //line 0
				});
			}
			else
			{
                //first
                glsl_config.m_rename_input_with_semantic = type == HLSL2ALL::ST_VERTEX_SHADER;
                //Apple bug
                glsl_config.m_rename_input_with_locations = glsl_config.m_rename_output_with_locations;
                glsl_config.m_input_prefix = glsl_config.m_output_prefix;
                glsl_config.m_rename_output_with_locations = type != HLSL2ALL::ST_COMPUTE_SHADER;
                glsl_config.m_output_prefix = std::string("__") + shader_target_name[type];
                //compile
				if (!HLSL2ALL::spirv_to_glsl(shader_spirv_out, shader_sources[type], shader_glsl_tslist, shader_spirv_errors, glsl_config))
				{
					context().add_wrong("Error to shader compile");
					context().add_wrongs(shader_spirv_errors);
					return false;
				}
				//remove #version
				extract_version_line(shader_sources[type], shader_headers[type]);
				//add inf
				shader_info.push_back
				(Render::ShaderSourceInformation
				{
					(Render::ShaderType)type //shader type
					, shader_headers[type]   //header
					, shader_sources[type]   //source output ref
					, 0						 //line 0
				});
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
            m_layouts = Render::Layout::Collection::layouts(render, m_shader);
            //ok
			return true;
		}
		return false;
	}

    const Shader::InputLayoutList& Shader::layouts() const
    {
        return m_layouts;
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
