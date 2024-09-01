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
#include "Square/Scene/Camera.h"

#define SPOT_LIGHT_DIR Vec3(0.0,0.0,-1.0)

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
		ctx.add_attributes<SpotLight>(attribute_function<SpotLight, bool>
		("visible"
		, bool(0)
		, [](const SpotLight* plight) -> bool       { return plight->visible(); }
		, [](SpotLight* plight, const bool& visible){ plight->visible(visible); }));

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
		//////////////////////////////////////////////////////////////////
		ctx.add_attributes<SpotLight>(attribute_function<SpotLight, Vec2>
		("shadow"
		, Vec2(0,0)
		, [](const SpotLight* plight) -> Vec2             { return plight->shadow_size(); }
		, [](SpotLight* plight, const Vec2& shadow_size)  { plight->shadow(shadow_size);  }));
		//////////////////////////////////////////////////////////////////

    }

	//light
	SpotLight::SpotLight(Context& context)
	: Component(context)
	, m_buffer(context)
	{
		m_sphere.radius(this->Render::SpotLight::radius());
		m_sphere.position({ 0,0,0 });
		m_position = Vec3(0, 0, 0.0);
		m_direction = SPOT_LIGHT_DIR;
	}

	//change radius aka change geometry
	void SpotLight::radius(const float light_r)
	{
		this->Render::SpotLight::radius(light_r);
		m_sphere.radius(this->Render::SpotLight::radius());
		//dirty
		m_frustum_is_dirty = true;
		m_viewport_is_dirty = false;
	}

	//change outer cut_off aka change frustum
	void SpotLight::outer_cut_off(const float coff)
	{
		//call light equivalent function
		this->Render::SpotLight::outer_cut_off(coff);
		//frustum dirty
		m_frustum_is_dirty = true;
		m_viewport_is_dirty = true;
	}

	//all events
	void SpotLight::on_attach(Actor& entity)
	{
		if (auto ptr_actor = actor().lock())
		{
			m_direction = to_mat3(ptr_actor->rotation(true)) * SPOT_LIGHT_DIR;
			m_position = ptr_actor->position(true);
			m_sphere.position(m_position);
			m_view_is_dirty = true;
			m_frustum_is_dirty = true;
		}
	}
	void SpotLight::on_deattch()
	{
		m_direction = SPOT_LIGHT_DIR;
		m_position = Vec3(0, 0, 0.0);
		m_sphere.position({ 0,0,0 });
		m_view_is_dirty = true;
		m_frustum_is_dirty = true;
	}
	void SpotLight::on_transform()
	{
		if (auto ptr_actor = actor().lock())
		{
			m_direction = to_mat3(ptr_actor->rotation(true)) * SPOT_LIGHT_DIR;
			m_position = ptr_actor->position(true);
			m_sphere.position(m_position);
			m_view_is_dirty = true;
			m_frustum_is_dirty = true;
		}
	}
	void SpotLight::on_message(const Message& msg){}

	//shadow
	const Render::ShadowBuffer& SpotLight::shadow_buffer() const
	{
		return m_buffer;
	}
	Vec4 SpotLight::shadow_viewport() const
	{
		projection(); //force update
		return m_viewport.viewport();
	}
	bool SpotLight::shadow() const
	{
		return m_buffer.width() != 0 && m_buffer.height() != 0;
	}
	void SpotLight::shadow(const IVec2& size)
	{
		if (m_buffer.size() != size)
		{
			if (size.x != 0 && size.y != 0)
				m_buffer.build(size, Render::ShadowBuffer::SB_TEXTURE_2D);
			else
				m_buffer.destoy();
			//dirty
			m_frustum_is_dirty = true;
			m_viewport_is_dirty = true;
		}
	}
	const IVec2& SpotLight::shadow_size() const
	{
		return m_buffer.size();
	}

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
		if (m_frustum_is_dirty)
		{
			//update frustum
			m_frustum.update_frustum(projection() * view());
			m_frustum_is_dirty = false;
		}
		return m_frustum;
	}
	Weak<Render::Transform> SpotLight::transform() const
	{
		return { DynamicPointerCast<Render::Transform>(actor().lock()) };
	}
	//methods
	void SpotLight::set(Render::UniformSpotLight* data) const
	{
		data->m_position = m_position;
		data->m_direction = m_direction;
		this->Render::SpotLight::set(data);
	}
	void SpotLight::set(Render::UniformSpotShadowLight* data) const
	{
		data->m_projection = projection();
		data->m_view = view();
	}
	//update spot light
	const Mat4& SpotLight::view() const
	{
		if (m_view_is_dirty)
		{
			if (auto ptr_actor = actor().lock())
			{
				m_view = Square::inverse(ptr_actor->global_model_matrix());
			}
			else
			{
				m_view = Constants::identity<Mat4>();
			}
			m_view_is_dirty = false;
		}
		return m_view;
	}
	const Mat4& SpotLight::projection() const
	{
		if (m_viewport_is_dirty)
		{
			//default aspect
			float aspect = 1.0;
			//shadow?
			if (shadow())
			{
				auto size = m_buffer.size();
				float aspect = size.x / size.y;
				m_viewport.viewport({ 0,0,size.x,size.y });
			}
			//update frustum
			m_viewport.perspective(m_outer_cut_off*1.99f, aspect, 0.1f, m_radius);
			//dirty
			m_viewport_is_dirty = false;
		}
		return m_viewport.projection();

	}
}
}