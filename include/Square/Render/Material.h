#pragma once
//
//  Light.h
//  Square
//
//  Created by Gabriele Di Bari on 10/04/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Object.h"
#include "Square/Render/Effect.h"


namespace Square
{
namespace Render
{
	class SQUARE_API Material : public BaseObject
	{
	public:

		SQUARE_OBJECT(Material)

		Material();

		Material(Shared<Effect> effect);

		virtual ~Material();
		
		Shared<Effect> effect() const;

		EffectParameters* parameters() const;

		virtual Shared<Material> copy() const;

		enum default_parameters
		{
			MAT_DEFAULT_DIFFUSE_MAP,
			MAT_DEFAULT_NORMAL_MAP,
			MAT_DEFAULT_SPECULAR_MAP,
			MAT_DEFAULT_COLOR,
			MAT_DEFAULT_MASK,
			MAT_DEFAULT_MAX
		};

		static std::string name_of_default_parameters[Material::MAT_DEFAULT_MAX];

		EffectParameter* default_parameter(default_parameters dp);
		const EffectParameter* default_parameter(default_parameters dp) const;

		EffectParameter* parameter_by_name(const std::string& name);
		const EffectParameter* parameter_by_name(const std::string& name) const;

		EffectQueueType queue() const;

	protected:

		//effect class
		Shared<Effect> m_effect;
		//effect parameters
		EffectParameters* m_parameters{ nullptr };
		//dafault params
		int m_default_parameters[MAT_DEFAULT_MAX]{ -1 };
	};
}
}
