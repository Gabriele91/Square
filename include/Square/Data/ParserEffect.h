#pragma once
#include "Square/Config.h"
#include "Square/Math/Linear.h"
#include "Square/Data/ParserUtils.h"
#include "Square/Driver/Render.h"
#include "Square/Render/Effect.h"
#include "Square/Data/ParserParameters.h"

namespace Square
{
namespace Parser
{
	class SQUARE_API Effect
	{

        using  ParametersMap   = ::Square::Render::EffectParametersMap;
        using  TechniquesMap   = ::Square::Render::EffectTechniquesMap;
        using  Technique       = ::Square::Render::EffectTechnique;
        using  Parameter       = ::Square::Render::EffectParameter;
        using  Parameters      = ::Square::Render::EffectParameters;
        using  ParameterType   = ::Square::Render::EffectParameterType;
        using  EffectQueueType = ::Square::Render::EffectQueueType;
		using  ParameterField  = Parser::Parameters::ParameterField;

	public:

        static std::string blend_to_string(Render::BlendType blend);
        
        static Render::BlendType blend_from_string(const std::string& blend, Render::BlendType blend_default);

        static std::string depth_to_string(Render::DepthFuncType depth);
        
        static Render::DepthFuncType depth_from_string(const std::string& depth, Render::DepthFuncType depth_default);

        static std::string cullface_to_string(Render::CullfaceType cullface);
        
        static Render::CullfaceType cullface_from_string(const std::string& cullface, Render::CullfaceType cullface_default);
        
		struct RequirementField
		{
			std::string m_driver_name{ "ALL" };
			int         m_driver_major_version;
			int         m_driver_minor_version;
			std::string m_shader_name{ "ALL" };
			int         m_shader_version;

            bool test(Render::Context* render) const;
		};

		struct ShaderField
		{
			bool        m_name{ false };
			std::string m_text;
			size_t      m_line{ 0 };
		};

		enum LightsField
		{
			LF_NONE = 0b00000,
			LF_BASE = 0b00001,
			LF_AMBIENT = 0b00010,
			LF_SPOT = 0b00100,
			LF_POINT = 0b01000,
			LF_DIRECTION = 0b10000,
			LF_SPOT_POINT = LF_SPOT | LF_POINT,
			LF_SPOT_DIRECTION = LF_SPOT | LF_DIRECTION,
			LF_POINT_DIRECTION = LF_POINT | LF_DIRECTION,
			LF_SPOT_POINT_DIRECTION = LF_SPOT | LF_POINT | LF_DIRECTION
		};

		struct PassField
		{
            Render::CullfaceState    m_cullface;
			Render::DepthBufferState m_depth;
			Render::BlendState       m_blend;
			ShaderField              m_shader;
			LightsField	             m_lights{ LF_BASE };
		};

		struct TechniqueField
		{
			std::string m_name;
			std::vector< PassField > m_pass;
		};

		struct SubEffectField
		{
			RequirementField m_requirement;
			EffectQueueType   m_queue;
			std::vector< TechniqueField > m_techniques;
		};

		struct Context
		{
			std::vector< SubEffectField >   m_sub_effect;
			std::vector< ParameterField >   m_parameters;
			std::list< ErrorField >	        m_errors;
			size_t						    m_line{ 1 };

            std::string errors_to_string() const;
		};

        bool parse(Context& default_context, const std::string& source);

        bool parse(Context& default_context, const char*& ptr);


	protected:
        //////////////////////////////////////////////////////
		Context* m_context{ nullptr };
		//////////////////////////////////////////////////////
		bool parse_parameters_block(const char*& ptr);
        //////////////////////////////////////////////////////
        bool parse_driver_type(const char*& ptr, RequirementField& field);
        bool parse_shader_type(const char*& ptr, RequirementField& field);
		//////////////////////////////////////////////////////
        bool parse_sub_effect_block(const char*& ptr);
		//////////////////////////////////////////////////////
        bool parse_requirement_block(const char*& ptr, RequirementField& r_field);
		//////////////////////////////////////////////////////
        bool parse_techniques_block(const char*& ptr, SubEffectField& subeffect);
		//////////////////////////////////////////////////////
        bool parse_pass_block(const char*& ptr, PassField& pass);
        bool parse_queue_block(const char*& ptr, EffectQueueType& p_queue);
		//////////////////////////////////////////////////////
        bool parse_blend(const char*& ptr, PassField& pass);
        bool parse_depth(const char*& ptr, PassField& pass);
        bool parse_cullface(const char*& ptr, PassField& pass);
        bool parse_lights(const char*& ptr, PassField& pass);
        bool parse_shader(const char*& ptr, PassField& pass);
		//////////////////////////////////////////////////////
        bool parse_queue_type(const char*& inout, Render::QueueType& type);
		//////////////////////////////////////////////////////
        void push_error(const std::string& error);

	};
}
}
