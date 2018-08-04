//
//  PointLight.cpp
//  Square
//
//  Created by Gabriele Di Bari on 27/04/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#include "Square/Core/Object.h"
#include "Square/Core/Context.h"
#include "Square/Core/ClassObjectRegistration.h"
#include "Square/Geometry/Sphere.h"
#include "Square/Geometry/Frustum.h"
#include "Square/Scene/Actor.h"
#include "Square/Scene/SpotLight.h"

namespace Square
{
namespace Scene
{
    //Add element to objects
    SQUARE_CLASS_OBJECT_REGISTRATION(SpotLight);
    
    //Registration in context
    void SpotLight::object_registration(Context& ctx)
    {
        //factory
        ctx.add_object<SpotLight>();
        //Attributes
		/* TODO */
		ctx.add_attributes<SpotLight>(attribute_function<SpotLight, Vec3>
		("diffuse"
		, Vec3(1.0)
		, [](const SpotLight* plight) -> Vec3       { return plight->diffuse(); }
		, [](SpotLight* plight, const Vec3& diffuse){ plight->diffuse(diffuse); }));

		ctx.add_attributes<SpotLight>(attribute_function<SpotLight, Vec3>
		("specular"
		, Vec3(1.0)
		, [](const SpotLight* plight) -> Vec3        { return plight->specular(); }
		, [](SpotLight* plight, const Vec3& specular){ plight->specular(specular); }));

		ctx.add_attributes<SpotLight>(attribute_function<SpotLight, float>
		("constant"
		, float(1.0)
		, [](const SpotLight* plight) -> float        { return plight->constant(); }
		, [](SpotLight* plight, const float& constant){ plight->constant(constant); }));

		ctx.add_attributes<SpotLight>(attribute_function<SpotLight, float>
		("radius"
		, float(1.0)
		, [](const SpotLight* plight) -> float        { return plight->radius(); }
		, [](SpotLight* plight, const float& radius)  { plight->radius(radius);  }));

		ctx.add_attributes<SpotLight>(attribute_function<SpotLight, float>
		("inside_radius"
		, float(1.0)
		, [](const SpotLight* plight) -> float               { return plight->inside_radius(); }
		, [](SpotLight* plight, const float& inside_radius)  { plight->inside_radius(inside_radius);  }));

		ctx.add_attributes<SpotLight>(attribute_function<SpotLight, float>
		("inner_cut_off"
		, float(1.0)
		, [](const SpotLight* plight) -> float               { return plight->inner_cut_off(); }
		, [](SpotLight* plight, const float& inner_cut_off)  { plight->inner_cut_off(inner_cut_off);  }));

		ctx.add_attributes<SpotLight>(attribute_function<SpotLight, float>
		("outer_cut_off"
		, float(1.0)
		, [](const SpotLight* plight) -> float               { return plight->outer_cut_off(); }
		, [](SpotLight* plight, const float& outer_cut_off)  { plight->outer_cut_off(outer_cut_off);  }));
    }

	//light
	SpotLight::SpotLight(Context& context)
	: Component(context)
	{
		m_sphere.radius(this->Render::SpotLight::radius());
		m_sphere.position({ 0,0,0 });
		m_position = Vec3(0, 0, 0.0);
		m_direction = Vec3(0, 0, 1.0);
	}
	//change radius aka change geometry
	void SpotLight::radius(const float light_r)
	{
		this->Render::SpotLight::radius(light_r);
		m_sphere.radius(this->Render::SpotLight::radius());
	}

	//all events
	void SpotLight::on_attach(Actor& entity)
	{
		if (auto ptr_actor = actor().lock())
		{
			m_direction = to_mat3(ptr_actor->rotation(true)) * Vec3(0, 0, 1.0);
			m_position = ptr_actor->position(true);
			m_sphere.position(m_position);
		}
	}
	void SpotLight::on_deattch()
	{
		m_direction = Vec3(0, 0, 1.0);
		m_position = Vec3(0, 0, 0.0);
		m_sphere.position({ 0,0,0 });
	}
	void SpotLight::on_transform()
	{
		if (auto ptr_actor = actor().lock())
		{
			m_direction = to_mat3(ptr_actor->rotation(true)) * Vec3(0, 0, 1.0);
			m_position = ptr_actor->position(true);
			m_sphere.position(m_position);
		}

	}
	void SpotLight::on_message(const Message& msg){}

	//object methods
	//serialize
	void SpotLight::serialize(Data::Archive& archivie)
	{
		Data::serialize(archivie, this);
	}
	void SpotLight::serialize_json(Data::Json& archivie)
	{ }
	//deserialize
	void SpotLight::deserialize(Data::Archive& archivie)
	{
		Data::deserialize(archivie, this);
	}
	void SpotLight::deserialize_json(Data::Json& archivie)
	{ }
	//methods
	const Geometry::Sphere& SpotLight::bounding_sphere() const
	{
		//return geometry
		return m_sphere;
	}
	const Geometry::Frustum& SpotLight::frustum() const
	{
		return {};
	}
	Weak<Render::Transform> SpotLight::transform() const
	{
		return
		{
			DynamicPointerCast<Render::Transform>(actor().lock())
		};
	}
	Weak<Render::Camera> SpotLight::camera() const
	{
		return {};
	}
	//methods
	void SpotLight::set(Render::UniformSpotLight* data) const
	{
		data->m_position = m_position;
		data->m_direction = m_direction;
		this->Render::SpotLight::set(data);
	}
}
}