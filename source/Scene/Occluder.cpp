//
//  Occluder.cpp
//  Square
//
//  See Occluder.h for the high level description.
//
#include <limits>
#include "Square/Config.h"
#include "Square/Scene/Actor.h"
#include "Square/Scene/Occluder.h"
#include "Square/Core/ClassObjectRegistration.h"

namespace Square
{
namespace Scene
{
	SQUARE_CLASS_OBJECT_REGISTRATION(Occluder);

	//regs
	void Occluder::object_registration(Square::Context& ctx)
	{
		ctx.add_object<Occluder>();
		// Its triangles
		ctx.add_attribute_function<Occluder, std::vector<Vec3> >
			("triangles"
			, std::vector<Vec3>()
			, [](const Occluder* occluder) -> std::vector<Vec3> { return occluder->m_triangles; }
			, [](Occluder* occluder, const std::vector<Vec3>& triangles) { occluder->triangles(triangles); });
		// Enabled
		ctx.add_attribute_function<Occluder, bool >
			("enabled"
			, true
			, [](const Occluder* occluder) -> bool { return occluder->m_enabled; }
			, [](Occluder* occluder, const bool& enabled) { occluder->enabled(enabled); });
	}

	Occluder::Occluder(Square::Context& context)
	: Component(context)
	{
	}

	Occluder::~Occluder()
	{
	}

	void Occluder::triangles(const std::vector<Vec3>& triangles)
	{
		m_triangles = triangles;
		m_dirty = true;
	}

	const std::vector<Vec3>& Occluder::occluder_triangles()
	{
		build_world();
		return m_world;
	}

	const Geometry::AABoundingBox& Occluder::occluder_box()
	{
		build_world();
		return m_world_box;
	}

	bool Occluder::can_occlude() const
	{
		return m_enabled && !m_triangles.empty();
	}

	void Occluder::build_world()
	{
		if (m_dirty)
		{
			Mat4 model(1.0f);
			if (auto transform = m_transform.lock())
			{
				model = transform->global_model_matrix();
			}
			Vec3 low(std::numeric_limits<float>::max());
			Vec3 high(std::numeric_limits<float>::lowest());
			m_world.clear();
			m_world.reserve(m_triangles.size());
			for (const Vec3& point : m_triangles)
			{
				const Vec3 world = Vec3(model * Vec4(point, 1.0f));
				m_world.push_back(world);
				low = glm::min(low, world);
				high = glm::max(high, world);
			}
			if (m_world.empty())
			{
				low = high = Vec3(0.0f);
			}
			m_world_box = Geometry::AABoundingBox(low, high);
			m_dirty = false;
		}
	}

	//events
	void Occluder::on_transform()
	{
		m_dirty = true;
	}

	void Occluder::on_attach(Actor& entity)
	{
		m_transform = DynamicPointerCast<Render::Transform>(entity.shared_from_this());
		m_dirty = true;
	}

	void Occluder::on_deattch()
	{
		m_transform.reset();
		m_dirty = true;
	}

	//serialize
	void Occluder::serialize(Data::Archive& archive)
	{
		Data::serialize(archive, this);
	}

	void Occluder::serialize_json(Data::JsonValue& archive)
	{
		Data::serialize_json(archive, this);
	}

	void Occluder::deserialize(Data::Archive& archive)
	{
		Data::deserialize(archive, this);
	}

	void Occluder::deserialize_json(Data::JsonValue& archive)
	{
		Data::deserialize_json(archive, this);
	}
}
}
