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
//Preprocess
#include "ShaderPreprocess.h"
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
		ctx.add_attribute_function<Shader>(
		("constant_buffer"
		, (void*)(nullptr)
		, [](const Shader* shader) -> void* { return (void*)shader->constant_buffer(name); }
		, [](Shader* shader, const void*&) { /*none*/ }
		));
        #endif
        //end
    }

	//Contructor
	Shader::Shader(Context& context) : ResourceObject(context), BaseInheritableSharedObject(context.allocator())
	{
	}
	Shader::Shader(Context& context, const std::string& path) : ResourceObject(context), BaseInheritableSharedObject(context.allocator())
	{
		load(path);
	}
	
	//Destructor
	Shader::~Shader()
	{
		destoy();
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
		//remove comments
		shader_remove_comments(preprocessed_source);
		//first pass, compute program
		if (!source_preprocess(preprocessed_source, defines, postprocessed_source)) return false;
		//if is HLSL
		if (postprocessed_source.m_hlsl_target) 
		{ 
			if (!hlsl_compile(postprocessed_source))  return false; 
		}
		//or GLSL
		else
		{
			//next to spirv
			if (!source_to_spirv(postprocessed_source, spirv)) return false;
			//to glsl
			if (!spirv_to_glsl_compile(postprocessed_source, spirv)) return false;
		}
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
			context().logger()->warning("Error to shader compile");
			context().logger()->warnings(shader_spirv_errors);
			return false;
		}	
		return true;
	}

	//SPIRV-Cross HLSL's backend implementation (tessellation, geometry and compute shaders) is unfinished.
	//So, do not use that backend.
	bool Shader::hlsl_compile
	(
		const PostprocessOutput& source
	)
	{
		////////////////////////////////////////////////////////////////////////////////////////////////////
		static const std::string shader_target_name[Render::ST_N_SHADER]
		{
			  "vertex"
			, "fragment"
			, "geometry"
			, "tass_control"
			, "tass_eval"
			, "compute"
		};
		//save types
		static const HLSL2ALL::InfoSpirvShaderList shader_info
		{
			{ HLSL2ALL::ST_VERTEX_SHADER, shader_target_name[Render::ST_VERTEX_SHADER] },
			{ HLSL2ALL::ST_TASSELLATION_CONTROL_SHADER, shader_target_name[Render::ST_TASSELLATION_CONTROL_SHADER] },
			{ HLSL2ALL::ST_TASSELLATION_EVALUATION_SHADER, shader_target_name[Render::ST_TASSELLATION_EVALUATION_SHADER] },
			{ HLSL2ALL::ST_GEOMETRY_SHADER, shader_target_name[Render::ST_GEOMETRY_SHADER] },
			{ HLSL2ALL::ST_FRAGMENT_SHADER, shader_target_name[Render::ST_FRAGMENT_SHADER] },
			{ HLSL2ALL::ST_COMPUTE_SHADER, shader_target_name[Render::ST_COMPUTE_SHADER] },
		};
		HLSL2ALL::ErrorSpirvShaderList shader_errors;
		//info
		HLSL2ALL::TargetShaderInfo spirv_info;
		spirv_info.m_client_version = 100;
		spirv_info.m_desktop = true;
		spirv_info.m_reverse_mul = false;
		spirv_info.m_vulkan = true;
		spirv_info.m_upgrade_texture_to_samples = false;
		spirv_info.m_samplerarray_to_flat = false;
		// Output
		HLSL2ALL::TypeHLSLShaderList shader_hlsl_outputs;
		//build
		if (!HLSL2ALL::hlsl_to_hlsl_preprocessed
		(
			  source.m_source
			, resource_name()
			, shader_info
			, shader_hlsl_outputs
			, shader_errors
			, spirv_info
			, true
		))
		{
			context().logger()->warning("Error to shader compile");
			context().logger()->warnings(shader_errors);
			return false;
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////
		std::vector< Render::ShaderSourceInformation > render_shader_info;
		////////////////////////////////////////////////////////////////////////////////////////////////////
		//as HLSL list
		for (size_t  idx = 0; idx < shader_hlsl_outputs.size(); ++idx)
		{			
			// Unpack
			Render::ShaderType type   = (Render::ShaderType)shader_hlsl_outputs[idx].m_type;
			std::string& hlslsource   = (std::string&)shader_hlsl_outputs[idx].m_shader;
			const std::string& target = shader_target_name[type];
			// Remove
			shader_remove_void_lines(hlslsource);
			//output
			render_shader_info.push_back
			(Render::ShaderSourceInformation
			{
				  type			     //shader type
				, ""				 //header
				, hlslsource         //source output ref
				, target             //source output ref
				, 0				     //line 0
			});
		}
		////////////////////////////////////////////////////////////////////////////////
		// load shaders from files
		if (auto render = context().render())
		{
			//compile
			m_shader = render->create_shader(render_shader_info);
			//tests
			if (!m_shader  || render->shader_compiled_with_errors(m_shader) || render->shader_linked_with_error(m_shader))
			{
				//fail
				context().logger()->warning("Error to shader compile");
				context().logger()->warnings(render->get_shader_compiler_errors(m_shader));
				context().logger()->warning(render->get_shader_linker_error(m_shader));
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
				context().logger()->warning("Error to shader compile");
				context().logger()->warnings(shader_spirv_errors);
				return false;
			}
			//remove #version
			shader_extract_glsl_version_line(shader_sources[type], shader_headers[type]);
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
				context().logger()->warning("Error to shader compile");
				context().logger()->warnings(render->get_shader_compiler_errors(m_shader));
				context().logger()->warning(render->get_shader_linker_error(m_shader));
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
		//clear info
		m_filepath_map.clear();
		m_uniform_map.clear();
		m_cbuffer_map.clear();
		//delete last shader
		if (m_shader)
			if (auto render = context().render())
				render->delete_shader(m_shader);
		m_shader = nullptr;
	}
}
}
