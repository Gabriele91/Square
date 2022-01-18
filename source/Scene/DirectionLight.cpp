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
#include "Square/Scene/DirectionLight.h"

#define DIRECTION_LIGHT_DIR Vec3(0.0,0.0,-1.0)

namespace Square
{
namespace Scene
{
    //Add element to objects
    SQUARE_CLASS_OBJECT_REGISTRATION(DirectionLight);
    
    //Registration in context
    void DirectionLight::object_registration(Context& ctx)
    {
        //factory
        ctx.add_object<DirectionLight>();
        //Attributes
		ctx.add_attributes<DirectionLight>(attribute_function<DirectionLight, bool>
		("visible"
		, bool(0)
		, [](const DirectionLight* plight) -> bool       { return plight->visible(); }
		, [](DirectionLight* plight, const bool& visible){ plight->visible(visible); }));

		ctx.add_attributes<DirectionLight>(attribute_function<DirectionLight, Vec3>
		("diffuse"
		, Vec3(1.0)
		, [](const DirectionLight* plight) -> Vec3       { return plight->diffuse(); }
		, [](DirectionLight* plight, const Vec3& diffuse){ plight->diffuse(diffuse); }));

		ctx.add_attributes<DirectionLight>(attribute_function<DirectionLight, Vec3>
		("specular"
		, Vec3(1.0)
		, [](const DirectionLight* plight) -> Vec3        { return plight->specular(); }
		, [](DirectionLight* plight, const Vec3& specular){ plight->specular(specular); }));
    }

	//light
	DirectionLight::DirectionLight(Context& context)
	: Component(context)
	, m_buffer(context)
	{
		m_direction = Vec3(0, 1.0, 0.0);
	}

	//all events
	void DirectionLight::on_attach(Actor& entity)
	{
		if (auto ptr_actor = actor().lock())
		{
			m_direction = to_mat3(ptr_actor->rotation(true)) * DIRECTION_LIGHT_DIR;
		}
	}
	void DirectionLight::on_deattch()
	{
		m_direction = DIRECTION_LIGHT_DIR;
	}
	void DirectionLight::on_transform()
	{
		if (auto ptr_actor = actor().lock())
		{
			m_direction = to_mat3(ptr_actor->rotation(true)) * DIRECTION_LIGHT_DIR;
		}
	}
	void DirectionLight::on_message(const Message& msg){}

	//Shadow
	const Render::ShadowBuffer& DirectionLight::shadow_buffer() const
	{
		return m_buffer;
	}
	bool DirectionLight::shadow() const
	{
		return false;
	}

	//object methods
	//serialize
	void DirectionLight::serialize(Data::Archive& archivie)
	{
		Data::serialize(archivie, this);
	}
	void DirectionLight::serialize_json(Data::Json& archivie)
	{ }
	//deserialize
	void DirectionLight::deserialize(Data::Archive& archivie)
	{
		Data::deserialize(archivie, this);
	}
	void DirectionLight::deserialize_json(Data::Json& archivie)
	{ }
	//methods
	const Geometry::Sphere& DirectionLight::bounding_sphere() const
	{
		return {};
	}
	const Geometry::Frustum& DirectionLight::frustum() const
	{
		return {};
	}
	Weak<Render::Transform> DirectionLight::transform() const
	{
		return { DynamicPointerCast<Render::Transform>(actor().lock()) };
	}
	//methods
	void DirectionLight::set(Render::UniformDirectionLight* data) const
	{
		data->m_direction = m_direction;
		this->Render::DirectionLight::set(data);
	}
}
}