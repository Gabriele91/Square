//  Square
//
//  Created by Gabriele on 15/08/16.
//  Copyright � 2016 Gabriele. All rights reserved.
//
//compiler HLSL2ALL
#define SQUARE_SHADER_COMPILER_FUNCTION_IMPL
#include <chrono>
#include <algorithm>
#include <HLSL2ALL/HLSL2ALL.h>
//Square includes
#include "Square/Config.h"
#include "Square/System/RenderSystem.h"
#include "Square/Core/Application.h"
#include "Square/Core/Filesystem.h"
#include "Square/Core/StringUtilities.h"
#include "Square/Core/Context.h"
#include "Square/Core/Attribute.h"
#include "Square/Data/ParserUtils.h"
#include "Square/Resource/Shader.h"
#include "Square/Resource/ShaderCache.h"
#include "Square/Render/Profiler.h"
#include "Square/Core/ClassObjectRegistration.h"
//Preprocess
#include "ShaderPreprocess.h"

namespace Square
{
namespace Resource
{
	namespace AuxShader
	{
		//a SPIR-V module without its debug info (the source in it, its lines: most of its size),
		//its names kept (the uniforms, the buffers bound by name)
		static HLSL2ALL::SpirvShader spirv_strip_debug(const HLSL2ALL::SpirvShader& words)
		{
			static constexpr size_t s_header = 5;
			static constexpr unsigned int s_op_source_continued = 2;
			static constexpr unsigned int s_op_source = 3;
			static constexpr unsigned int s_op_source_extension = 4;
			static constexpr unsigned int s_op_string = 7;
			static constexpr unsigned int s_op_line = 8;
			static constexpr unsigned int s_op_no_line = 317;
			static constexpr unsigned int s_op_module_processed = 330;
			HLSL2ALL::SpirvShader out;
			if (words.size() >= s_header)
			{
				out.reserve(words.size());
				out.insert(out.end(), words.begin(), words.begin() + s_header);
				size_t at = s_header;
				while (at < words.size())
				{
					const unsigned int opcode = words[at] & 0xFFFF;
					const size_t count = std::max<size_t>(words[at] >> 16, 1);
					const size_t end = std::min(at + count, words.size());
					switch (opcode)
					{
					case s_op_source_continued:
					case s_op_source:
					case s_op_source_extension:
					case s_op_string:
					case s_op_line:
					case s_op_no_line:
					case s_op_module_processed:
						break;
					default:
						out.insert(out.end(), words.begin() + at, words.begin() + end);
						break;
					}
					at = end;
				}
			}
			return out;
		}

		//a source as a key: without its #line (the files of the machine that loaded it: a path of
		//Windows, none on Android, DISABLE_SHARER_PREPROCESS_LINE_SET) and its empty lines (the ones
		//around the #line); the same source, the same key on every machine
		static String key_source(const String& source)
		{
			static constexpr const char   s_line[] = "#line";
			static constexpr const size_t s_line_length = sizeof(s_line) - 1;
			String out;
			out.reserve(source.size());
			size_t start = 0;
			while (start < source.size())
			{
				size_t end = source.find('\n', start);
				if (end == String::npos)
				{
					end = source.size();
				}
				const size_t first = source.find_first_not_of(" \t\r", start);
				const bool empty = first == String::npos || first >= end;
				const bool line = !empty && source.compare(first, s_line_length, s_line) == 0;
				if (!empty && !line)
				{
					out.append(source, start, end - start);
					out += '\n';
				}
				start = end + 1;
			}
			return out;
		}

		//the location of each input of a vertex stage of Vulkan by its semantic: the attribute of the
		//input layouts (Render::AttributeType), as DirectX matches them by semantic
		static Vector< std::pair<String, unsigned int> > vertex_input_locations()
		{
			return
			{
				{ "POSITION0", Render::ATT_POSITION0 }, { "NORMAL0", Render::ATT_NORMAL0 },
				{ "TEXCOORD0", Render::ATT_TEXCOORD0 }, { "TANGENT0", Render::ATT_TANGENT0 },
				{ "BINORMAL0", Render::ATT_BINORMAL0 }, { "COLOR0", Render::ATT_COLOR0 },
				{ "POSITION1", Render::ATT_POSITION1 }, { "NORMAL1", Render::ATT_NORMAL1 },
				{ "TEXCOORD1", Render::ATT_TEXCOORD1 }, { "TANGENT1", Render::ATT_TANGENT1 },
				{ "BINORMAL1", Render::ATT_BINORMAL1 }, { "COLOR1", Render::ATT_COLOR1 },
				{ "NORMAL2", Render::ATT_NORMAL2 }, { "TEXCOORD2", Render::ATT_TEXCOORD2 },
				{ "TANGENT2", Render::ATT_TANGENT2 }, { "BINORMAL2", Render::ATT_BINORMAL2 },
				{ "COLOR2", Render::ATT_COLOR2 }
			};
		}
	}

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
	Shader::Shader(Context& context, const String& path) : ResourceObject(context), BaseInheritableSharedObject(context.allocator())
	{
		load(path);
	}
	
	//Destructor
	Shader::~Shader()
	{
		destoy();
	}
	//load shader
	bool Shader::load(const String& path) 
	{
		//load & compile
		return compile(path, Filesystem::text_file_read_all(path), PreprocessMap());
	}

	bool Shader::load(const String& path, const PreprocessMap& defines)
	{
		//load & compile
		return  compile(path, Filesystem::text_file_read_all(path), defines);
	}

	//compile from source
	bool Shader::compile
	(
		const String& insource,
		const PreprocessMap& defines
	)
	{
		return compile(Filesystem::resource_dir(), insource, defines);
	}

	//compile
    bool Shader::compile
	(
		const String& path,
        const String& insource,
        const PreprocessMap& defines,
        const size_t line
	)
    {
		//delete last shader
		if (m_shader) destoy();
#if defined(SQUARE_DEBUG_TOOLS)
		const auto compile_start = std::chrono::high_resolution_clock::now();
		m_driver_ms = 0.0;
		struct Timed
		{
			Shader& m_shader;
			String m_name;
			std::chrono::high_resolution_clock::time_point m_start;
			~Timed()
			{
				//the translation (preprocess, HLSL -> SPIR-V -> target, or the cache), the driver apart
				const double total = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - m_start).count();
				m_shader.context().add_load_detail("shader: translate", m_name, std::max(total - m_shader.m_driver_ms, 0.0));
				m_shader.context().add_load_detail("shader: driver", m_name, m_shader.m_driver_ms);
			}
		};
		Timed timed
		{ 
			*this, 
			*resource_name() ? String(resource_name()) : Filesystem::get_filename(path), 
			compile_start 
		};
#endif
		//allocs
		String       preprocessed_source;
		PostprocessOutput postprocessed_source;
		//process include/import
		ShaderImportLoader preprocess(context(), insource, path, m_filepath_map, preprocessed_source, true, line);
		//fail
		if (!preprocess.success()) return false;
		//remove comments
		shader_remove_comments(preprocessed_source);
		//the target: the driver (a tool: each one of a platform)
		auto render_system = System::get<RenderSystem>(context());
		const ShaderTarget& target = render_system->shader_target();
		//first pass, compute program
		if (!source_preprocess(preprocessed_source, defines, target, postprocessed_source)) return false;
		//its stages: from the intermediate cache (the pack, the folder of the runtime), else
		//translated and kept there
		const String language = target.name();
		const ShaderCache::Key key = ShaderCache::key(cache_key(postprocessed_source, target));
		const ShaderCache& cache = render_system->shader_cache();
		ShaderStages stages;
		//(a release with a pack has all of them: one not there, a pack of other shaders or of
		//another target, from the folder of the runtime, else translated here)
		if (!cache.pack().empty() && !cache.in_pack(language, key))
		{
			const String name = *resource_name() ? String(resource_name()) : Filesystem::get_filename(path);
			context().logger()->warning("Shader: " + name + " not in the pack " + cache.pack() + " (" + language + ")");
		}
		if (!cache.find(language, key, stages))
		{
			if (!translate(postprocessed_source, stages)) return false;
			cache.store(language, key, stages);
		}
		//the shader of the driver (its own final cache); a tool: the stages only
		return target.m_translate_only || create_from(stages);
    }

	String Shader::cache_key(const PostprocessOutput& source, const ShaderTarget& target) const
	{
		//what makes the stages: the target (its language, its version), the version of the
		//converter, the source preprocessed (its includes, its defines, the common header in it:
		//the backend, the gamma correction); never the driver nor the files of the machine (a pack
		//serves every machine)
		String what = target.name();
		what += "|" + to_string(source.m_version);
		what += source.m_texture_target ? "|texture" : "|screen";
		what += "|cache" + to_string(ShaderCache::version);
		what += "|" + AuxShader::key_source(source.m_source);
		return what;
	}

	bool Shader::translate(const PostprocessOutput& source, ShaderStages& stages)
	{
		bool translated = false;
		//HLSL (DirectX): preprocessed
		if (source.m_language == ShaderTarget::L_HLSL)
		{
			translated = hlsl_compile(source, stages);
		}
		//or MSL (Metal), GLSL, GLSL ES, SPIR-V (Vulkan): by SPIR-V
		else
		{
			HLSL2ALL::TypeSpirvShaderList spirv;
			translated = source_to_spirv(source, spirv);
			if (translated)
			{
				switch (source.m_language)
				{
				case ShaderTarget::L_MSL:
				case ShaderTarget::L_MSL_IOS:
					translated = spirv_to_msl_compile(source, spirv, stages);
					break;
				case ShaderTarget::L_SPIRV:
					translated = spirv_stages(spirv, stages);
					break;
				case ShaderTarget::L_GLSL:
				case ShaderTarget::L_GLSL_ES:
				default:
					translated = spirv_to_glsl_compile(source, spirv, stages);
					break;
				}
			}
		}
		return translated;
	}

	bool Shader::create_from(const ShaderStages& stages)
	{
		//the input of the driver (its strings: the stages)
		Vector< Render::ShaderSourceInformation > infos;
		infos.reserve(stages.size());
		for (const ShaderStage& stage : stages)
		{
			infos.push_back(Render::ShaderSourceInformation{ stage.m_type, stage.m_header, stage.m_source, stage.m_entry, 0 });
		}
		bool created = false;
		if (auto render = System::get<RenderSystem>(context())->render())
		{
			m_shader = create_shader(*render, infos);
			if (!m_shader || render->shader_linked_with_error(m_shader))
			{
				context().logger()->error("Error to shader compile");
				context().logger()->errors(render->get_shader_compiler_errors(m_shader));
				context().logger()->error(render->get_shader_linker_error(m_shader));
			}
			else
			{
				if (render->shader_compiled_with_errors(m_shader))
				{
					//maybe it is just a warning
					context().logger()->warning("Wrong shader compile");
					context().logger()->warnings(render->get_shader_compiler_errors(m_shader));
					context().logger()->warning(render->get_shader_linker_error(m_shader));
				}
				created = true;
			}
		}
		return created;
	}

	bool Shader::source_preprocess
	(
		const String& raw_source,
		const PreprocessMap& defines,
		const ShaderTarget& target,
		PostprocessOutput& source
	)
	{
		//the target: its language, its version (an effect can ask another one: "version")
		source.m_language = target.m_language;
		source.m_version = size_t(target.m_version);
		source.m_texture_target = false;
        //commond header: the conventions of the backend (GL: depth in [-1,1], rows up)
		String shader_commond_header = "#pragma pack_matrix( row_major )\n";
		switch (target.m_language)
		{
		case ShaderTarget::L_HLSL:    shader_commond_header += "#define SQ_BACKEND_HLSL\n";  break;
		case ShaderTarget::L_MSL:
		case ShaderTarget::L_MSL_IOS: shader_commond_header += "#define SQ_BACKEND_MSL\n";   break;
		case ShaderTarget::L_SPIRV:   shader_commond_header += "#define SQ_BACKEND_SPIRV\n"; break;
		case ShaderTarget::L_GLSL_ES: shader_commond_header += "#define SQ_BACKEND_GLSL\n#define SQ_BACKEND_GLSL_ES\n"; break;
		case ShaderTarget::L_GLSL:
		default:                      shader_commond_header += "#define SQ_BACKEND_GLSL\n";  break;
		}
		// sRGB / gamma correction
		if (target.m_gamma)
		{
			// Texture decode (sRGB->linear) is always done by the shader via to_rgb_space().
			shader_commond_header += "#define ENABLE_GAMMA_CORRECTION 1\n";
			// Output encode (linear->sRGB): let the hardware do it only when the
			// framebuffer is actually sRGB-capable (DX UNORM_SRGB or GL with sRGB FB).
			// Otherwise the shader handles it via to_srgb_space() as a fallback.
			if (target.m_srgb_target)
			{
				shader_commond_header += "#define ENABLE_TARGET_SRGB 1\n";
			}
		}
        //add commond header
		shader_commond_header +=
		#include "ShaderCommonHeader.hlsl"
		;
		//list define
		String header_string;
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
		String shader_target_name[Render::ST_N_SHADER]
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
		spirv_info.m_upgrade_texture_to_samples = true;
		//SPIRV-Cross currently does not support flattening structs recursively.
		spirv_info.m_samplerarray_to_flat = false;
		//the SPIR-V of the driver of Vulkan: one binding for each resource of the program, the
		//inputs of the vertex stage at the location of their attribute (Render::AttributeType)
		if (source.m_language == ShaderTarget::L_SPIRV)
		{
			spirv_info.m_vulkan_layout = true;
			spirv_info.m_vertex_input_locations = AuxShader::vertex_input_locations();
		}
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
		const PostprocessOutput& source,
		ShaderStages& stages
	)
	{
		////////////////////////////////////////////////////////////////////////////////////////////////////
		static const String shader_target_name[Render::ST_N_SHADER]
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
		//as HLSL list
		for (size_t  idx = 0; idx < shader_hlsl_outputs.size(); ++idx)
		{			
			// Unpack
			Render::ShaderType type   = (Render::ShaderType)shader_hlsl_outputs[idx].m_type;
			String& hlslsource   = (String&)shader_hlsl_outputs[idx].m_shader;
			const String& target = shader_target_name[type];
			// Remove
			shader_remove_void_lines(hlslsource);
			//output
			stages.push_back(ShaderStage{ type, target, "", hlslsource });
		}
		////////////////////////////////////////////////////////////////////////////////
		// load shaders from files
		return true;
	}

	bool Shader::spirv_to_glsl_compile
	(
		const PostprocessOutput& source,
		const HLSL2ALL::TypeSpirvShaderList& shader_spirv_outputs,
		ShaderStages& stages
	)
	{
		////////////////////////////////////////////////////////////////////////////////////////////////////
		String shader_target_name[Render::ST_N_SHADER]
		{
			  "vertex"
			, "fragment"
			, "geometry"
			, "tass_control"
			, "tass_eval"
			, "compute"
		};
		//compile
		String shader_headers[Render::ST_N_SHADER];
		String shader_sources[Render::ST_N_SHADER];
        //config
        HLSL2ALL::GLSLConfig glsl_config;
		glsl_config.m_version = int(source.m_version);
		glsl_config.m_es = source.m_language == ShaderTarget::L_GLSL_ES;
		glsl_config.m_vulkan_semantics = false;
		glsl_config.m_rename_position_in_position0 = true;
		glsl_config.m_fixup_clipspace = true;
		glsl_config.m_flip_vert_y = source.m_texture_target; //flip y if is a texture target
		//(the baseline: no optional extension of a driver, the same translation on every machine)
		glsl_config.m_enable_420pack_extension = false;
        glsl_config.m_force_to_remove_query_texture = true;
		//force all uniform-buffer bindings to 0 (blocks are re-bound by name on the host)
		glsl_config.m_force_uniform_buffer_binding_zero = true;
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
            glsl_config.m_output_prefix = String("__") + shader_target_name[type];
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
			stages.push_back(ShaderStage{ (Render::ShaderType)type, "main", shader_headers[type], shader_sources[type] });
		}

#ifdef SQUARE_DEBUG_TOOLS
		//the GLSL made, in a folder (SQUARE_DUMP_GLSL): what the driver compiles, by stage, named
		//by the hash of the vertex source (the stages of a shader together)
		if (const char* dump = std::getenv("SQUARE_DUMP_GLSL"))
		{
			const size_t id = std::hash<String>()(shader_sources[Render::ST_VERTEX_SHADER] + shader_sources[Render::ST_FRAGMENT_SHADER]);
			for (const ShaderStage& stage : stages)
			{
				const String file = Filesystem::join(dump, to_string(id) + "." + shader_target_name[stage.m_type] + ".glsl");
				Filesystem::text_file_write_all(file, stage.m_header + stage.m_source);
			}
		}
#endif

		////////////////////////////////////////////////////////////////////////////////
		// load shaders from files
		return true;
	}

	bool Shader::spirv_to_msl_compile
	(
		const PostprocessOutput& source,
		const HLSL2ALL::TypeSpirvShaderList& shader_spirv_outputs,
		ShaderStages& stages
	)
	{
		String shader_target_name[Render::ST_N_SHADER]
		{
			  "vertex"
			, "fragment"
			, "geometry"
			, "tass_control"
			, "tass_eval"
			, "compute"
		};

		String shader_sources[Render::ST_N_SHADER];

		HLSL2ALL::MSLConfig msl_config;
		msl_config.m_ios   = source.m_language == ShaderTarget::L_MSL_IOS;
		msl_config.m_macos = !msl_config.m_ios;
		msl_config.m_msl_version = int(source.m_version); // 20000: MSL 2.0
		// SPIR-V uses Vulkan/DX Z [0,w] == Metal Z; fixup_clipspace would wrongly re-remap it, releasing behind-camera vertices.
		msl_config.m_fixup_clipspace = false;

		HLSL2ALL::ErrorSpirvShaderList spirv_errors;

		for (const HLSL2ALL::TypeSpirvShader& ssoutput : shader_spirv_outputs)
		{
			int  type            = ssoutput.m_type;
			auto& shader_spirv   = const_cast<HLSL2ALL::SpirvShader&>(ssoutput.m_shader);

			if (!HLSL2ALL::spirv_to_msl(shader_spirv, shader_sources[type], spirv_errors, msl_config))
			{
				context().logger()->warning("Error converting SPIR-V to MSL");
				context().logger()->warnings(spirv_errors);
				return false;
			}

			stages.push_back(ShaderStage{ (Render::ShaderType)type, shader_target_name[type], "", shader_sources[type] });
		}

		return true;
	}

	bool Shader::spirv_stages
	(
		const HLSL2ALL::TypeSpirvShaderList& shader_spirv_outputs,
		ShaderStages& stages
	)
	{
		static const String shader_target_name[Render::ST_N_SHADER]
		{
			  "vertex"
			, "fragment"
			, "geometry"
			, "tass_control"
			, "tass_eval"
			, "compute"
		};
		for (const HLSL2ALL::TypeSpirvShader& ssoutput : shader_spirv_outputs)
		{
			//its words without the debug info, as bytes (the entry: the function of the stage)
			const HLSL2ALL::SpirvShader words = AuxShader::spirv_strip_debug(ssoutput.m_shader);
			String bytes(reinterpret_cast<const char*>(words.data()), words.size() * sizeof(words[0]));
			stages.push_back(ShaderStage{ (Render::ShaderType)ssoutput.m_type, shader_target_name[ssoutput.m_type], "", std::move(bytes) });
		}
		return true;
	}

	Render::Shader* Shader::create_shader(Render::Context& render, const Vector< Render::ShaderSourceInformation >& stages)
	{
#if defined(SQUARE_DEBUG_TOOLS)
		const auto start = std::chrono::high_resolution_clock::now();
		Render::Shader* shader = render.create_shader(stages);
		m_driver_ms += std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - start).count();
		return shader;
#else
		return render.create_shader(stages);
#endif
	}

    //get buffer
    Render::Uniform* Shader::uniform(const String& name) const
    {
        //(none without the shader of the driver: a tool translates only)
        if (auto render = System::get<RenderSystem>(context())->render(); render && m_shader)
        {
            return render->get_uniform(m_shader, String(name));
        }
        return nullptr;
    }
    
    Render::UniformConstBuffer* Shader::constant_buffer(const String& name) const
    {
        if (auto render = System::get<RenderSystem>(context())->render(); render && m_shader)
        {
            return render->get_uniform_const_buffer(m_shader, name);
        }
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
		if (auto render = System::get<RenderSystem>(context())->render())
		{
			render->bind_shader(m_shader);
			SQUARE_RENDER_SHADER(*render, profile_name().c_str());
		}
	}

	//name in the profiler
	const String& Shader::profile_name() const
	{
		if (m_profile_name.empty())
		{
			//the resource name without its type ("Shader:SSAO" -> "SSAO")
			const String name = resource_name();
			const size_t colon = name.find(':');
			m_profile_name = colon == String::npos ? name : name.substr(colon + 1);
			if (m_profile_name.empty()) m_profile_name = "(unnamed)";
		}
		return m_profile_name;
	}

	void Shader::profile_name(const String& name)
	{
		m_profile_name = name;
	}

	//unbind shader
	void Shader::unbind()
	{
		if (auto render = System::get<RenderSystem>(context())->render())
		{
			render->unbind_shader(m_shader);
			SQUARE_RENDER_SHADER(*render, nullptr);
		}
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
			if (auto render = System::get<RenderSystem>(context())->render())
				render->delete_shader(m_shader);
		m_shader = nullptr;
	}
}
}
