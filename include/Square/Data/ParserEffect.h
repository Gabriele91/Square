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

        static String blend_to_string(Render::BlendType blend);
        
        static Render::BlendType blend_from_string(const String& blend, Render::BlendType blend_default);

        static String depth_to_string(Render::DepthFuncType depth);
        
        static Render::DepthFuncType depth_from_string(const String& depth, Render::DepthFuncType depth_default);

        static String cullface_to_string(Render::CullfaceType cullface);
        
        static Render::CullfaceType cullface_from_string(const String& cullface, Render::CullfaceType cullface_default);
        
		// Capability gate: ANY ignores the capability, REQUIRE matches only drivers
		// that have it, EXCLUDE matches only drivers that lack it.
		enum CapabilityTest
		{
			CAP_ANY = 0,
			CAP_REQUIRE,
			CAP_EXCLUDE
		};

		struct RequirementField
		{
			String    m_driver_name{ "ALL" };
			int            m_driver_major_version{ 0 };
			int            m_driver_minor_version{ 0 };
			String    m_shader_name{ "ALL" };
			int            m_shader_version{ 0 };
			CapabilityTest m_geometry_shader{ CAP_ANY };
			CapabilityTest m_vertex_viewport_index{ CAP_ANY };
			CapabilityTest m_draw_instanced{ CAP_ANY };

            //the driver (the one of the target of the shaders: Resource::ShaderTarget) meets it
            bool test(const Render::RenderDriverInfo& info) const;
		};

		struct ShaderField
		{
			enum Type
			{
				S_NONE,
				S_SOURCE,
				S_RESOUCE,
				S_INCLUDE
			};
			//shader type
			Type m_type{ S_NONE };
			//include
			String m_data;
			size_t      m_line{ 0 };
		};

		enum LightsField : unsigned int
		{
			LT_NONE                 = 0b000000,
			LT_COLOR                = 0b000001,
			LT_AMBIENT              = 0b000010,
			LT_SPOT                 = 0b000100,
			LT_POINT                = 0b001000,
			LT_DIRECTION            = 0b010000,
			LT_AREA                 = 0b100000,
			LT_SPOT_POINT           = LT_SPOT | LT_POINT,
			LT_SPOT_DIRECTION       = LT_SPOT | LT_DIRECTION,
			LT_POINT_DIRECTION      = LT_POINT| LT_DIRECTION,
			LT_SPOT_POINT_DIRECTION = LT_SPOT | LT_POINT | LT_DIRECTION
		};

		struct PassField
		{
            //fixed, or param(name): from a parameter of the effect
            Render::PassState< Render::CullfaceState >    m_cullface;
			Render::PassState< Render::DepthBufferState > m_depth;
			Render::PassState< Render::BlendState >       m_blend;
			ShaderField              m_shader;
			LightsField	             m_lights{ LT_COLOR }; //enable lights
			LightsField				 m_shadows{ LT_NONE  }; //enable shadows
			int                      m_draw_count{ 1 };    //multi-pass: times to draw
			int                      m_instances{ 1 };     //instanced draw: instance count per draw
		};

		struct TechniqueField
		{
			String m_name;
			Vector< PassField > m_pass;
			//"variants instanced clip": the Render::EffectVariant declared (each combination is compiled)
			unsigned char m_variants{ Render::EV_NONE };
			//"clip(mask, dither)": the parameters that turn the clip variant on (none: only the fade
			//of a level of detail)
			Vector< String > m_clip_parameters;
		};

		struct ImportField
		{
			//type
			enum Type
			{
				I_RESOUCE,
				I_INCLUDE,
				I_FROM_RESOUCE,
				I_FROM_INCLUDE
			};
			Type m_type{ I_RESOUCE };
			//effect path/name
			String m_data;
			//effect technique name
			String m_technique_name;
		};

		struct SubEffectField
		{
			RequirementField m_requirement;
			EffectQueueType   m_queue;
			Vector< TechniqueField > m_techniques;
			Vector< ImportField > m_imports;
		};

		struct Context
		{
			Vector< SubEffectField >   m_sub_effect;
			Vector< ParameterField >   m_parameters;
			List< ErrorField >	        m_errors;
			size_t						    m_line{ 1 };

            String errors_to_string() const;
		};

        bool parse(Allocator* allocator, Context& default_context, const String& source);

        bool parse(Allocator* allocator, Context& default_context, const char*& ptr);


	protected:
        //////////////////////////////////////////////////////
		Context* m_context{ nullptr };
		Allocator* m_allocator{ nullptr };
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
        bool parse_variants(const char*& ptr, TechniqueField& technique);
        bool parse_clip_parameters(const char*& ptr, TechniqueField& technique);
		//////////////////////////////////////////////////////
		bool parse_import(const char*& ptr, SubEffectField& subeffect);
		bool parse_from(const char*& ptr, SubEffectField& subeffect);
		//////////////////////////////////////////////////////
        bool parse_pass_block(const char*& ptr, PassField& pass);
        bool parse_queue_block(const char*& ptr, EffectQueueType& p_queue);
		//////////////////////////////////////////////////////
        bool parse_blend(const char*& ptr, PassField& pass);
        bool parse_depth(const char*& ptr, PassField& pass);
        bool parse_cullface(const char*& ptr, PassField& pass);
        //param(name): a parametric state, its parameter
        bool parse_state_param(const char*& ptr, String& name);
        bool parse_lights(const char*& ptr, PassField& pass);
		bool parse_shadows(const char*& ptr, PassField& pass);
        bool parse_draw_count(const char*& ptr, PassField& pass);
        bool parse_instances(const char*& ptr, PassField& pass);
        bool parse_shader(const char*& ptr, PassField& pass);
		//////////////////////////////////////////////////////
        bool parse_queue_type(const char*& inout, Render::QueueType& type);
		//////////////////////////////////////////////////////
        void push_error(const String& error);

	};
}
}
