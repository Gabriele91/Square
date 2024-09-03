//
//  Effect.cpp
//  Square
//
//  Created by Gabriele Di Bari on 09/04/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#include "Square/Config.h"
#include "Square/Core/Context.h"
#include "Square/Core/Filesystem.h"
#include "Square/Core/ClassObjectRegistration.h"
#include "Square/Render/Effect.h"
#include "Square/Render/Material.h"
#include "Square/Resource/Effect.h"
#include "Square/Resource/Material.h"
#include "Square/Resource/Texture.h"
#include "Square/Data/ParserMaterial.h"

namespace Square
{
namespace Resource
{
	//////////////////////////////////////////////////////////////
	//Add element to objects
	SQUARE_CLASS_OBJECT_REGISTRATION(Material);
	//Registration in context
	void Material::object_registration(Context& ctx)
	{
		//factory
		ctx.add_resource<Material>({ ".mat" });
	}
	//////////////////////////////////////////////////////////////
	//constructor
	Material::Material(Context& context) : ResourceObject(context), BaseInheritableSharedObject(context.allocator()) {}
	Material::Material(Context& context, const std::string& path) : ResourceObject(context), BaseInheritableSharedObject(context.allocator()) { load(path); }
	//help
	static void save_default_parameter(int id_param, const std::string& name, int default_parameters[])
	{
		for (int i = 0; i != Material::MAT_DEFAULT_MAX; ++i)
		{
			if (Material::name_of_default_parameters[i] == name)
			{
				default_parameters[i] = id_param;
				return;
			}
		}
	}
	//load effect
	bool Material::load(const std::string& path)
	{
		//parse
		Parser::Material mat_parser;
		Parser::Material::Context mat_context;
		//do parsing
		if (!mat_parser.parse(context().allocator(), mat_context, Filesystem::text_file_read_all(path)))
		{
			context().logger()->warning("Material: " + path + "\n" + mat_context.errors_to_string());
			return false;
		}
		//load effect
		m_effect = context().resource<Effect>(mat_context.m_effect);
		//load params
		if (m_effect)
		{
			//copy context
			m_parameters = m_effect->copy_all_parameters();
			//set local params
			for (size_t i = 0; i != mat_context.m_parameters.size(); ++i)
			{
				//get field
				Parser::Parameters::ParameterField& ctx_param = mat_context.m_parameters[i];
				//get id
				int  id_param = m_effect->parameter_id(ctx_param.m_name);
				//test
				if (id_param < 0) continue;
				//get pram
				Render::EffectParameter* param = (*m_parameters)[id_param].get();
				//select
				switch (ctx_param.m_type)
				{
				//textures
				case Render::EffectParameterType::PT_TEXTURE: param->set(context().resource<Resource::Texture>(ctx_param.m_resources[0])); break;
				case Render::EffectParameterType::PT_STD_VECTOR_TEXTURE:
				{
					//vector of texture
					std::vector< Shared<Resource::Texture> > textures;
					//get all texture
					for (const std::string& tex_name : ctx_param.m_resources) textures.push_back(context().resource<Resource::Texture>(tex_name));
					//set
					param->set(textures);
				}
				//default
				default: 
					if(ctx_param.m_paramter->get_type() == param->get_type())
						ctx_param.m_paramter->copy_to(param);  
				break;
				}
				//if a default parameter
				save_default_parameter(id_param, ctx_param.m_name, m_default_parameters);
			}
		}
		return m_effect.get() != nullptr;
	}

}
}
