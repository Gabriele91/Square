#pragma once

#pragma once
#include "Square/Config.h"
#include "Square/Core/Resource.h"
#include "Square/Driver/Render.h"

namespace Square
{
namespace Resource
{
	class SQUARE_API Shader : public ResourceObject
                            , public SharedObject<Shader>
	{

		Render::Shader*       m_shader{ nullptr };

	public:
        //Init object
        SQUARE_OBJECT(Shader)
        //Registration in context
        static void object_registration(Context& ctx);
        
        //Contructor
		Shader(const std::string& path);

		//Destructor
		virtual ~Shader();

		//load shader
		bool load(Context& context, const std::string& path) override;

		bool load(const std::string& path);

		//get buffer
		Render::ConstBuffer* constant_buffer(const char *name) const;

		//bind shader
		virtual void bind();

		//unbind shader
		virtual void unbind();

		//destoy shader
		void destoy();

	};
}
}