#include "Square/Render/Material.h"

namespace Square
{
namespace Render
{
	std::string Material::name_of_default_parameters[Material::MAT_DEFAULT_MAX]
	{
		"diffuse_map",
		"normal_map",
		"specular_map",
		"color",
		"mask"
	};

	Material::Material() {}
	Material::Material(Shared<Effect> effect)
	{
		//effect
		m_effect = effect;
		//copy context
		m_parameters = m_effect->copy_all_parameters();
		//set default params
		for (unsigned int i = 0; i != Material::MAT_DEFAULT_MAX; ++i)
		{
			//get param id
			int param_id = m_effect->parameter_id(name_of_default_parameters[i]);
			//save
			if (param_id != -1) m_default_parameters[i] = param_id;
		}
	}
	Material::~Material() { if (m_parameters) delete m_parameters; };
	
	Shared<Effect>	  Material::effect() const { return m_effect; }
	EffectParameters* Material::parameters() const { return m_parameters; }

	EffectParameter* Material::default_parameter(Material::default_parameters dp)
	{
		int id_param = m_default_parameters[dp];
		if (id_param >= 0) return (*m_parameters)[m_default_parameters[dp]].get();
		return nullptr;
	}
	const EffectParameter* Material::default_parameter(Material::default_parameters dp) const
	{
		int id_param = m_default_parameters[dp];
		if (id_param >= 0) return (*m_parameters)[m_default_parameters[dp]].get();
		return nullptr;
	}

	EffectParameter* Material::parameter_by_name(const std::string& name)
	{
		if (!m_effect) return nullptr;
		//get param
		int id_param = m_effect->parameter_id(name);
		if (id_param >= 0) return (*m_parameters)[id_param].get();
		return nullptr;
	}
	const EffectParameter* Material::parameter_by_name(const std::string& name) const
	{
		if (!m_effect) return nullptr;
		//get param
		int   id_param = m_effect->parameter_id(name);
		if (id_param >= 0) return (*m_parameters)[id_param].get();
		return nullptr;
	}
	
	Shared<Material> Material::copy(Allocator* allocator) const
	{
		auto omaterial = MakeShared<Material>(allocator);
		//effect
		omaterial->m_effect = m_effect;
		//parameters alloc
		omaterial->m_parameters = new EffectParameters(m_parameters->size());
		//copy
		for (size_t i = 0; i != m_parameters->size(); ++i)
		{
			(*omaterial->m_parameters)[i] = Unique< EffectParameter >((*m_parameters)[i]->copy());
		}
		//return
		return omaterial;
	}

	EffectQueueType Material::queue() const
	{
		return m_effect ? m_effect->queue() : EffectQueueType();
	}
}
}