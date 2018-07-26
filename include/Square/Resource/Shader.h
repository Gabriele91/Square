#pragma once
#include <string>
#include <unordered_map>
#include "Square/Config.h"
#include "Square/Core/Resource.h"
#include "Square/Driver/Render.h"
#include "Square/Render/VertexLayout.h"

namespace Square
{
namespace Resource
{
	class SQUARE_API Shader : public ResourceObject
                            , public SharedObject<Shader>
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
        
        //load from effect
        bool load(const std::string& path,
                  const std::string& source,
                  const PreprocessMap& defines,
                  const size_t line = 0);
        
		//compile from source
		bool compile
		(
			const std::string& source,
			const PreprocessMap& defines
		);

		//get buffer
		Render::Uniform*            uniform(const std::string& name) const;
		Render::UniformConstBuffer* constant_buffer(const std::string& name) const;

		//RAW Shader
		Render::Shader* base_shader() const;
        
        //Input layout
        const InputLayoutList& layouts() const;
        
		//bind shader
		virtual void bind();

		//unbind shader
		virtual void unbind();

		//destoy shader
		void destoy();

	protected:

		//Values
		Render::Shader* m_shader{ nullptr };
		FilepathMap	    m_filepath_map;
		UnifomMap	    m_uniform_map;
		CBufferMap	    m_cbuffer_map;
        //layouts
        InputLayoutList m_layouts;

	};
}
}
