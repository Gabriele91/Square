#pragma once
#include <string>
#include <unordered_map>
#include "Square/Config.h"
#include "Square/Core/Resource.h"
#include "Square/Driver/Render.h"
#include "Square/Resource/ShaderCache.h"
#include "Square/Resource/ShaderTarget.h"
#include "Square/Render/VertexLayout.h"

namespace Square
{
namespace Resource
{
	class SQUARE_API Shader : public ResourceObject
                            , public InheritableSharedObject<Shader>
	{

	public:
		//Types
		using PreprocessElement = std::tuple< std::string, std::string >;
		using PreprocessMap     = std::vector< PreprocessElement >;
		using FilepathMap       = std::unordered_map< size_t, std::string >;
		using UnifomMap         = std::unordered_map< std::string, std::string >;
		using CBufferMap        = std::unordered_map< std::string, std::string >;
        using InputLayoutList   = Render::Layout::InputLayoutList;

        //Init object
        SQUARE_OBJECT(Shader)
        //Registration in context
        static void object_registration(Context& ctx);
        
        //Contructor
		Shader(Context& context);
		Shader(Context& context, const std::string& path);

		//Destructor
		virtual ~Shader();

		//load shader
        bool load(const std::string& path) override;
        
        //load + defiles
        bool load(const std::string& path, const PreprocessMap& defines);
		        
		//compile from source
		bool compile
		(
			const std::string& source,
			const PreprocessMap& defines
		);
		bool compile
		(
			const std::string& path,					 
			const std::string& source,
			const PreprocessMap& defines,
			const size_t line = 0
		);
		//get buffer
		Render::Uniform*            uniform(const std::string& name) const;
		Render::UniformConstBuffer* constant_buffer(const std::string& name) const;

		//RAW Shader
		Render::Shader* base_shader() const;

		//bind shader
		virtual void bind();

		//unbind shader
		virtual void unbind();

		//destoy shader
		void destoy();

		//the name of the shader in the render profiler: the resource name (without its type), or the
		//one given (the shaders of the effects are not resources)
		const std::string& profile_name() const;
		void profile_name(const std::string& name);

	protected:

		//the name in the profiler (made by profile_name() when empty)
		mutable std::string m_profile_name;

		//Values
		Render::Shader* m_shader{ nullptr };
		//the shader of the driver from its stages (with the debug tools: its time, the one of the
		//driver, in the details of the loads)
		Render::Shader* create_shader(Render::Context& render, const std::vector< Render::ShaderSourceInformation >& stages);
#if defined(SQUARE_DEBUG_TOOLS)
		double m_driver_ms{ 0.0 };
#endif
		FilepathMap	    m_filepath_map;
		UnifomMap	    m_uniform_map;
		CBufferMap	    m_cbuffer_map;

		// help to compile
		struct PostprocessOutput
		{
			ShaderTarget::Language m_language;
			size_t	    m_version;
			bool	    m_texture_target;
			std::string m_header;
			std::string m_source;
			std::string m_raw_source;
		};

		bool source_preprocess
		(
			const std::string& sourcecode,
			const PreprocessMap& defines,
			const ShaderTarget& target,
			PostprocessOutput& source
		);

		//what makes the stages of a source (the key of the intermediate cache: its target, its
		//version, the source preprocessed)
		std::string cache_key(const PostprocessOutput& source, const ShaderTarget& target) const;
		//its stages in the language of the target (HLSL -> SPIR-V -> GLSL / GLSL ES / MSL / SPIR-V,
		//or HLSL)
		bool translate(const PostprocessOutput& source, ShaderStages& stages);
		//the shader of the driver from its stages (its errors in the log)
		bool create_from(const ShaderStages& stages);

		#ifdef SQUARE_SHADER_COMPILER_FUNCTION_IMPL
		bool hlsl_compile
		(
			const PostprocessOutput& source,
			ShaderStages& stages
		);

		bool source_to_spirv
		(
			const PostprocessOutput& source,
			HLSL2ALL::TypeSpirvShaderList& output
		);

		bool spirv_to_glsl_compile
		(
			const PostprocessOutput& source,
			const HLSL2ALL::TypeSpirvShaderList& input,
			ShaderStages& stages
		);

		bool spirv_to_msl_compile
		(
			const PostprocessOutput& source,
			const HLSL2ALL::TypeSpirvShaderList& input,
			ShaderStages& stages
		);

		//SPIR-V (Vulkan): its words, the bytes of the source of each stage
		bool spirv_stages
		(
			const HLSL2ALL::TypeSpirvShaderList& input,
			ShaderStages& stages
		);
		#endif
	};
}
}
