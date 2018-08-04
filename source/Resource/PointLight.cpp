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
#include "Square/Scene/PointLight.h"

namespace Square
{
namespace Scene
{
    //Add element to objects
    SQUARE_CLASS_OBJECT_REGISTRATION(PointLight);
    
    //Registration in context
    void PointLight::object_registration(Context& ctx)
    {
        //factory
        ctx.add_object<PointLight>();
        //Attributes
		/* TODO */
		ctx.add_attributes<PointLight>(attribute_function<PointLight, Vec3>
		("diffuse"
		, Vec3(1.0)
		, [](const PointLight* plight) -> Vec3       { return plight->diffuse(); }
		, [](PointLight* plight, const Vec3& diffuse){ plight->diffuse(diffuse); }));

		ctx.add_attributes<PointLight>(attribute_function<PointLight, Vec3>
		("specular"
		, Vec3(1.0)
		, [](const PointLight* plight) -> Vec3        { return plight->specular(); }
		, [](PointLight* plight, const Vec3& specular){ plight->specular(specular); }));

		ctx.add_attributes<PointLight>(attribute_function<PointLight, float>
		("constant"
		, float(1.0)
		, [](const PointLight* plight) -> float        { return plight->constant(); }
		, [](PointLight* plight, const float& constant){ plight->constant(constant); }));

		ctx.add_attributes<PointLight>(attribute_function<PointLight, float>
		("radius"
		, float(1.0)
		, [](const PointLight* plight) -> float        { return plight->radius(); }
		, [](PointLight* plight, const float& radius)  { plight->radius(radius);  }));

		ctx.add_attributes<PointLight>(attribute_function<PointLight, float>
		("inside_radius"
		, float(1.0)
		, [](const PointLight* plight) -> float               { return plight->inside_radius(); }
		, [](PointLight* plight, const float& inside_radius)  { plight->inside_radius(inside_radius);  }));
    }

	//light
	PointLight::PointLight(Context& context)
	: Component(context)
	{
		m_sphere.radius(this->Render::PointLight::radius());
		m_sphere.position({ 0,0,0 });
		m_position = Vec3(0, 0, 0.0);
	}
	//change radius aka change geometry
	void PointLight::radius(const float light_r)
	{
		this->Render::PointLight::radius(light_r);
		m_sphere.radius(this->Render::PointLight::radius());
	}

	//all events
	void PointLight::on_attach(Actor& entity)
	{
		if (auto ptr_actor = actor().lock())
		{
			m_position = ptr_actor->position(true);
			m_sphere.position(m_position);
		}
	}
	void PointLight::on_deattch()
	{
		m_position = Vec3(0, 0, 0);
		m_sphere.position({ 0,0,0 });
	}
	void PointLight::on_transform()
	{
		if (auto ptr_actor = actor().lock())
		{
			m_position = ptr_actor->position(true);
			m_sphere.position(m_position);
		}

	}
	void PointLight::on_message(const Message& msg){}

	//object methods
	//serialize
	void PointLight::serialize(Data::Archive& archivie)
	{
		Data::serialize(archivie, this);
	}
	void PointLight::serialize_json(Data::Json& archivie)
	{ }
	//deserialize
	void PointLight::deserialize(Data::Archive& archivie)
	{
		Data::deserialize(archivie, this);
	}
	void PointLight::deserialize_json(Data::Json& archivie)
	{ }
	//methods
	const Geometry::Sphere& PointLight::bounding_sphere() const
	{
		//return geometry
		return m_sphere;
	}
	const Geometry::Frustum& PointLight::frustum() const
	{
		return {};
	}
	Weak<Render::Transform> PointLight::transform() const
	{
		return{ DynamicPointerCast<Render::Transform>(actor().lock()) };
	}
	Weak<Render::Camera> PointLight::camera() const
	{
		return {};
	}
	//methods
	void PointLight::set(Render::UniformPointLight* data) const
	{
		data->m_position = m_position;
		this->Render::PointLight::set(data);
	}
}
}