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
		//load & compile
		return compile(path, Filesystem::text_file_read_all(path), PreprocessMap());
	}

	bool Shader::load(const std::string& path, const PreprocessMap& defines)
	{
		//load & compile
		return  compile(path, Filesystem::text_file_read_all(path), defines);
	}

	//compile from source
	bool Shader::compile
	(
		const std::string& insource,
		const PreprocessMap& defines
	)
	{
		return compile(Filesystem::resource_dir(), insource, defines);
	}

	//compile
    bool Shader::compile
	(
		const std::string& path,
        const std::string& insource,
        const PreprocessMap& defines,
        const size_t line
	)
    {
		//delete last shader
		if (m_shader) destoy();
		//allocs
		std::string                   preprocessed_source;
		PostprocessOutput             postprocessed_source;
		HLSL2ALL::TypeSpirvShaderList spirv;
		//process include/import
		ShaderImportLoader preprocess(context(), insource, path, m_filepath_map, preprocessed_source, true, line);
		//fail
		if (!preprocess.success()) return false;
		//first pass, compute program
		if (!source_preprocess(preprocessed_source, defines, postprocessed_source)) return false;
		//next to spirv
		if (!source_to_spirv(postprocessed_source, spirv)) return false;
		//if is hlsl
		if (postprocessed_source.m_hlsl_target) { if (!spirv_to_hlsl_compile(postprocessed_source, spirv)) return false; }
		else								    { if (!spirv_to_glsl_compile(postprocessed_source, spirv)) return false; }
        //success
        return true;
    }


	bool Shader::source_preprocess
	(
		const std::string& raw_source,
		const PreprocessMap& defines,
		PostprocessOutput& source
	)
	{	
		//int shader version
		source.m_hlsl_target = false;
		source.m_version = 410;
		source.m_texture_target = false;
		//OpenGL or DirectX
		if (auto render = context().render())
		{
			source.m_hlsl_target = render->get_render_driver_info().m_shader_language == "HLSL";
		}
        //commond header
		std::string               shader_commond_header = "#pragma pack_matrix( row_major )\n";
		if (source.m_hlsl_target) shader_commond_header+= "#define HLSL_BACKEND\n";
		else		              shader_commond_header+= "#define GLSL_BACKEND\n";
        //add commond header
		shader_commond_header +=
		#include "ShaderCommonHeader.hlsl"
		;
		//list define
		std::string header_string;
		//defines
		for (auto& p : defines)
		{
			//get version
			if (std::get<0>(p) == "version")
			{
				int version = std::atoi(std::get<1>(p).c_str());
				if (version > 0) source.m_version = version;
				continue;
			}
			//get is target
			if (std::get<0>(p) == "pragma")
			{
				     if (std::get<1>(p) == "target_texture") source.m_texture_target = true;
				else if (std::get<1>(p) == "target_screen")  source.m_texture_target = false;
				continue;
			}
			//add
			header_string += "#" + std::get<0>(p) + " " + std::get<1>(p) + "\n";
		}
		//end source
		source.m_header = shader_commond_header + header_string;
		source.m_source = source.m_header + raw_source;
		source.m_raw_source = raw_source;
		////////////////////////////////////////////////////////////////////////////////
		return true;
	}

	bool Shader::source_to_spirv
	(
		const PostprocessOutput& source,
		HLSL2ALL::TypeSpirvShaderList& shader_spirv_outputs
	)
	{		
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
		spirv_info.m_client_version = 100;
		spirv_info.m_desktop = true;
		spirv_info.m_reverse_mul = false;
		spirv_info.m_vulkan = true;
		spirv_info.m_upgrade_texture_to_samples = false;
		//SPIRV-Cross currently does not support flattening structs recursively.
		spirv_info.m_samplerarray_to_flat = false;
		//build
		if (!HLSL2ALL::hlsl_to_spirv
		(
			  source.m_source
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
		return true;
	}
	//SPIRV-Cross HLSL's backend implementation (tessellation, geometry and compute shaders) is unfinished.
	//So, do not use that backend.
	bool Shader::spirv_to_hlsl_compile
	(
		const PostprocessOutput& source,
		const HLSL2ALL::TypeSpirvShaderList& shader_spirv_outputs
	)
	{
		////////////////////////////////////////////////////////////////////////////////////////////////////
		std::string shader_target_name[Render::ST_N_SHADER]
		{
			  "vertex"
			, "fragment"
			, "geometry"
			, "tass_control"
			, "tass_eval"
			, "compute"
		};
		////////////////////////////////////////////////////////////////////////////////////////////////////
		std::vector< Render::ShaderSourceInformation > shader_info;
		////////////////////////////////////////////////////////////////////////////////////////////////////
		//to HLSL/GLSL
		for (const HLSL2ALL::TypeSpirvShader& ssoutput : shader_spirv_outputs)
		{
			//unpack
            int   type = ssoutput.m_type;
            auto& shader_spirv_out = ssoutput.m_shader;
			//output
			shader_info.push_back
			(Render::ShaderSourceInformation
			{
				 (Render::ShaderType)type  //shader type
				, source.m_header	       //header
				, source.m_raw_source	   //source output ref
				, shader_target_name[type] //source output ref
				, 0						   //line 0
			});
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
            //ok
			return true;
		}
        //fail
        return false;
	}

	bool Shader::spirv_to_glsl_compile
	(
		const PostprocessOutput& source,
		const HLSL2ALL::TypeSpirvShaderList& shader_spirv_outputs
	)
	{
		////////////////////////////////////////////////////////////////////////////////////////////////////
		std::string shader_target_name[Render::ST_N_SHADER]
		{
			  "vertex"
			, "fragment"
			, "geometry"
			, "tass_control"
			, "tass_eval"
			, "compute"
		};
		////////////////////////////////////////////////////////////////////////////////////////////////////
		std::vector< Render::ShaderSourceInformation > shader_info;
		////////////////////////////////////////////////////////////////////////////////////////////////////
		//compile
		std::string shader_headers[Render::ST_N_SHADER];
		std::string shader_sources[Render::ST_N_SHADER];
        //config
        HLSL2ALL::GLSLConfig glsl_config;
		glsl_config.m_version = source.m_version;
		glsl_config.m_es = false;
		glsl_config.m_vulkan_semantics = false;
		glsl_config.m_rename_position_in_position0 = true;
		glsl_config.m_fixup_clipspace = true;
		glsl_config.m_flip_vert_y = source.m_texture_target; //flip y if is a texture target
		glsl_config.m_enable_420pack_extension = false;
        glsl_config.m_force_to_remove_query_texture = true;		
		//errors
		HLSL2ALL::TextureSamplerList shader_glsl_tslist;
		HLSL2ALL::ErrorSpirvShaderList shader_spirv_errors;
		//to HLSL/GLSL
		for (const HLSL2ALL::TypeSpirvShader& ssoutput : shader_spirv_outputs)
		{
			//unpack
            int   type = ssoutput.m_type;
            auto& shader_spirv_out = ssoutput.m_shader;
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
            //ok
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
