//
//  PointLight.cpp
//  Square
//
//  Created by Gabriele Di Bari on 27/04/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#include "Square/Core/Object.h"
#include "Square/Core/Context.h"
#include "Square/Math/Transformation.h"
#include "Square/Core/ClassObjectRegistration.h"
#include "Square/Render/ShadowBuffer.h"
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
		ctx.add_attribute_function<PointLight, bool>
		("visible"
		, bool(0)
		, [](const PointLight* plight) -> bool       { return plight->visible(); }
		, [](PointLight* plight, const bool& visible){ plight->visible(visible); });

		ctx.add_attribute_function<PointLight, Vec3>
		("diffuse"
		, Vec3(1.0)
		, [](const PointLight* plight) -> Vec3       { return plight->diffuse(); }
		, [](PointLight* plight, const Vec3& diffuse){ plight->diffuse(diffuse); });

		ctx.add_attribute_function<PointLight, Vec3>
		("specular"
		, Vec3(1.0)
		, [](const PointLight* plight) -> Vec3        { return plight->specular(); }
		, [](PointLight* plight, const Vec3& specular){ plight->specular(specular); });

		ctx.add_attribute_function<PointLight, float>
		("constant"
		, float(1.0)
		, [](const PointLight* plight) -> float        { return plight->constant(); }
		, [](PointLight* plight, const float& constant){ plight->constant(constant); });

		ctx.add_attribute_function<PointLight, float>
		("radius"
		, float(1.0)
		, [](const PointLight* plight) -> float        { return plight->radius(); }
		, [](PointLight* plight, const float& radius)  { plight->radius(radius);  });

		ctx.add_attribute_function<PointLight, float>
		("inside_radius"
		, float(1.0)
		, [](const PointLight* plight) -> float               { return plight->inside_radius(); }
		, [](PointLight* plight, const float& inside_radius)  { plight->inside_radius(inside_radius);  });
		
		ctx.add_attribute_function<PointLight, IVec2>
		("shadow"
		, IVec2(0)
		, [](const PointLight* plight) -> IVec2 { return plight->shadow_size(); }
		, [](PointLight* plight, const IVec2& shadow_size)  { plight->shadow(shadow_size);  });
		
    }

	////////////////////////////////////////
	// build view matrix
	static Mat4 cube_view_matrix(const Vec3& pos, size_t i)
	{
		//(+x -x +y -y +z -z)^-1
		switch (i)
		{
		case 0: return look_at(pos, pos + Vec3(1, 0, 0), Vec3(0, -1, 0));
		case 1: return look_at(pos, pos + Vec3(-1, 0, 0), Vec3(0, -1, 0));

		case 2: return look_at(pos, pos + Vec3(0, 1, 0), Vec3(0, 0, 1));
		case 3: return look_at(pos, pos + Vec3(0, -1, 0), Vec3(0, 0, -1));

		case 4: return look_at(pos, pos + Vec3(0, 0, 1), Vec3(0, -1, 0));
		case 5: return look_at(pos, pos + Vec3(0, 0, -1), Vec3(0, -1, 0));
		default: return  Constants::identity<Mat4>();
		}
	}
	//light
	PointLight::PointLight(Context& context)
	: Component(context)
	, SharedObject<PointLight>(context.allocator())
	, m_buffer(context)
	{
		m_sphere.radius(this->Render::PointLight::radius());
		m_sphere.position({ 0,0,0 });
		m_position = Vec3(0, 0, 0.0);
		//build views
		for (size_t i = 0; i != 6; ++i)
		{
			m_views[i] = cube_view_matrix(Vec3(0), i);
		}
	}
	//change radius aka change geometry
	void PointLight::radius(const float light_r)
	{
		this->Render::PointLight::radius(light_r);
		m_sphere.radius(this->Render::PointLight::radius());
		m_projection_is_dirty = true;
	}

	//all events
	void PointLight::on_attach(Actor& entity)
	{
		if (auto ptr_actor = actor().lock())
		{
			m_position = ptr_actor->position(true);
			m_sphere.position(m_position);
			m_views_is_dirty = true;
		}
	}
	void PointLight::on_deattch()
	{
		m_position = Vec3(0, 0, 0);
		m_sphere.position({ 0,0,0 });
		m_views_is_dirty = true;
	}
	void PointLight::on_transform()
	{
		if (auto ptr_actor = actor().lock())
		{
			m_position = ptr_actor->position(true);
			m_sphere.position(m_position);
			m_views_is_dirty = true;
		}

	}
	void PointLight::on_message(const Message& msg){}

	//shadow
	const Render::ShadowBuffer& PointLight::shadow_buffer() const
	{
		return m_buffer;
	}
	Vec4 PointLight::shadow_viewport() const
	{
		return {0, 0, m_buffer.width(), m_buffer.height()};
	}
	bool PointLight::shadow() const
	{
		return m_buffer.width() != 0 && m_buffer.height() != 0;
	}
	void PointLight::shadow(const IVec2& size)
	{
		if (m_buffer.size() != size)
		{
			if (size.x != 0 && size.y != 0)
				m_buffer.build(size, Render::ShadowBuffer::SB_TEXTURE_CUBE);
			else
				m_buffer.destoy();
		}
		m_projection_is_dirty = true;
	}
	const IVec2& PointLight::shadow_size() const
	{
		return m_buffer.size();
	}

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
	//methods
	void PointLight::set(Render::UniformPointLight* data) const
	{
		data->m_position = m_position;
		this->Render::PointLight::set(data);
	}
	void PointLight::set(Render::UniformPointShadowLight* data, bool draw_shadow_map) const
	{
		//projection 
		data->m_projection = projection();
		//views (cube)
		for (size_t i = 0; i != 6; ++i)
		{
			data->m_view[i] = views(i);
		}
		//world position
		data->m_position = Vec4(m_position,1);
		//range
		data->m_radius = radius();
	}	
	
	//update spot light
	const Mat4& PointLight::views(size_t i) const
	{
		if (m_views_is_dirty)
		{
			//apply
			for (size_t i = 0; i != 6; ++i)
			{
				m_views[i] = cube_view_matrix(m_position, i);
			}
			//disable flag
			m_views_is_dirty = false;
		}
		return m_views[i];
	}
	const Mat4& PointLight::projection() const
	{
		if (m_projection_is_dirty)
		{
			//default aspect
			float aspect = 1.0;
			//shadow?
			if (shadow())
			{
				auto size = m_buffer.size();
				float aspect = float(size.x) / size.y;
			}
			//update projection
			m_projection = perspective(Constants::pi<float>() / 2.0f, aspect, 0.1f, m_radius);
			//disable flag
			m_projection_is_dirty = false;
		}
		return m_projection;

	}}
}