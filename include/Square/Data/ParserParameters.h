#pragma once
#include "Square/Config.h"
#include "Square/Math/Linear.h"
#include "Square/Data/ParserUtils.h"
#include "Square/Driver/Render.h"
#include "Square/Render/Effect.h"

namespace Square
{
namespace Parser
{
	class SQUARE_API Parameters
	{
        using  Parameter      = ::Square::Render::EffectParameter;
        using  ParameterType  = ::Square::Render::EffectParameterType;

	public:

		struct ParameterField
		{
            //info
			std::string     m_name;
            ParameterType   m_type;
            Parameter*      m_paramter{ nullptr };
            //resource
            std::vector< std::string > m_resource;
            //by type
            bool alloc(ParameterType type);
            bool alloc(const std::string& name, ParameterType type);
            //alloc
            template < class T >
            bool alloc()
            {
                return alloc(::Square::Render::parameter_type_traits<T>());
            }
            template < class T >
            bool alloc(const std::string& name)
            {
                return alloc(name,::Square::Render::parameter_type_traits<T>());
            }
            //alloc
            template < class T >
            bool set(const std::string& name, const T& value)
            {
                //if
                if(!m_paramter && !alloc<T>(name)) return false;
                //save
                m_paramter->set(value);
                //else
                return true;
            }
			//default
			ParameterField();
			//copy / move
			ParameterField(ParameterField&& value);
			ParameterField(const ParameterField& value);
			//dealloc
            virtual ~ParameterField();
		};
		
		struct Context
		{
			std::vector< ParameterField >   m_parameters;
			std::list< ErrorField >	        m_errors;
			size_t						    m_line{ 0 };

            std::string errors_to_string() const;
		};

        bool parse(Context& default_context, const std::string& source);

        bool parse(Context& default_context, const char*& ptr);

	protected:
        //////////////////////////////////////////////////////
		Context* m_context{ nullptr };
		//////////////////////////////////////////////////////
        bool parse_parameters_block(const char*& ptr);
        bool parse_value(const char*& ptr, ParameterField& field);
        bool parse_int_values(const char*& ptr, int* values, size_t n);
        bool parse_float_values(const char*& ptr, float* values, size_t n);
        bool parse_double_values(const char*& ptr, double* values, size_t n);
        bool parse_texture(const char*& ptr, ParameterField& field);
        bool parse_mat3(const char*& ptr, Mat3& field);
        bool parse_mat4(const char*& ptr, Mat4& field);
        bool parse_dmat3(const char*& ptr, DMat3& field);
        bool parse_dmat4(const char*& ptr, DMat4& field);
		//////////////////////////////////////////////////////       
		bool parse_type(const char*& inout, ParameterType& type);
		//////////////////////////////////////////////////////       
        void push_error(const std::string& error);
	};

}
}
