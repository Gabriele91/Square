#include "Square/Config.h"
#include "Square/Scene/Actor.h"
#include "Square/Scene/LodGroup.h"
#include "Square/Scene/StaticMesh.h"
#include "Square/Render/Camera.h"
#include "Square/Geometry/OBoundingBox.h"
#include "Square/Geometry/AABoundingBox.h"
#include "Square/Core/ClassObjectRegistration.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace Square
{
namespace Scene
{
	SQUARE_CLASS_OBJECT_REGISTRATION(LodGroup);

	//regs
	void LodGroup::object_registration(Square::Context& ctx)
	{
		ctx.add_object<LodGroup>();
		//how a level is chosen (LodMode)
		ctx.add_attribute_function<LodGroup, int>
		("mode"
		, int(LodMode::SCREEN)
		, [](const LodGroup* group) -> int      { return int(group->mode()); }
		, [](LodGroup* group, const int& mode)  { group->mode(LodMode(mode)); });
		//its levels: their children, their thresholds
		ctx.add_attribute_function<LodGroup, std::vector<std::string> >
		("actors"
		, std::vector<std::string>()
		, [](const LodGroup* group) -> std::vector<std::string>
		{
			std::vector<std::string> actors;
			actors.reserve(group->levels().size());
			for (const LodGroup::Level& level : group->levels())
			{
				actors.push_back(level.m_actor);
			}
			return actors;
		}
		, [](LodGroup* group, const std::vector<std::string>& actors)
		{
			std::vector<LodGroup::Level> levels = group->levels();
			levels.resize(actors.size());
			for (size_t i = 0; i != actors.size(); ++i)
			{
				levels[i].m_actor = actors[i];
			}
			group->levels(levels);
		});
		ctx.add_attribute_function<LodGroup, std::vector<float> >
		("thresholds"
		, std::vector<float>()
		, [](const LodGroup* group) -> std::vector<float>
		{
			std::vector<float> thresholds;
			thresholds.reserve(group->levels().size());
			for (const LodGroup::Level& level : group->levels())
			{
				thresholds.push_back(level.m_threshold);
			}
			return thresholds;
		}
		, [](LodGroup* group, const std::vector<float>& thresholds)
		{
			std::vector<LodGroup::Level> levels = group->levels();
			levels.resize(std::max(levels.size(), thresholds.size()));
			for (size_t i = 0; i != thresholds.size(); ++i)
			{
				levels[i].m_threshold = thresholds[i];
			}
			group->levels(levels);
		});
		//its bounds
		ctx.add_attribute_function<LodGroup, Vec3>
		("center"
		, Vec3(0.0f)
		, [](const LodGroup* group) -> Vec3         { return group->center(); }
		, [](LodGroup* group, const Vec3& center)   { group->bounds(center, group->size()); });
		ctx.add_attribute_function<LodGroup, float>
		("size"
		, float(0.0f)
		, [](const LodGroup* group) -> float        { return group->size(); }
		, [](LodGroup* group, const float& size)    { group->bounds(group->center(), size); });
	}

	LodGroup::LodGroup(Square::Context& context)
	: Component(context)
	{
	}

	void LodGroup::mode(LodMode mode)
	{
		m_mode = mode;
	}

	void LodGroup::levels(const std::vector<Level>& levels)
	{
		m_levels = levels;
		refresh();
	}

	void LodGroup::bounds(const Vec3& center, float size)
	{
		m_center = center;
		m_size = size;
	}

	namespace AuxLodGroupBounds
	{
		//a box (min, max) grown by the 8 corners of another, through a matrix
		static void grow(Vec3& low, Vec3& high, const Geometry::AABoundingBox& box, const Mat4& model)
		{
			const Vec3& a = box.get_min();
			const Vec3& b = box.get_max();
			for (int corner = 0; corner != 8; ++corner)
			{
				const Vec3 local((corner & 1) ? b.x : a.x, (corner & 2) ? b.y : a.y, (corner & 4) ? b.z : a.z);
				const Vec3 point = Vec3(model * Vec4(local, 1.0f));
				low = min(low, point);
				high = max(high, point);
			}
		}
	}

	bool LodGroup::recalculate_bounds()
	{
		auto owner = actor().lock();
		if (!owner) return false;
		if (!found()) find();
		//the boxes of its renderables in the space of its actor: a static mesh by the corners of
		//the box of its mesh (exact, no decomposition), another by its box in the world
		const Mat4 to_group = inverse(owner->global_model_matrix());
		const float infinity = std::numeric_limits<float>::infinity();
		Vec3 low(infinity);
		Vec3 high(-infinity);
		for (const Renderables& renderables : m_renderables)
		{
			for (const auto& weak_renderable : renderables)
			{
				auto renderable = weak_renderable.lock();
				if (!renderable || !renderable->support_culling()) continue;
				auto transform = renderable->transform().lock();
				auto mesh = DynamicPointerCast<StaticMesh, Render::Renderable>(renderable);
				if (mesh && transform && mesh->local_bounding_box().valid())
				{
					const Mat4 to_mesh = to_group * transform->global_model_matrix();
					AuxLodGroupBounds::grow(low, high, mesh->local_bounding_box().to_aabb(), to_mesh);
				}
				else
				{
					AuxLodGroupBounds::grow(low, high, renderable->bounding_box().to_aabb(), to_group);
				}
			}
		}
		const Vec3 sides = high - low;
		const float size = std::max({ sides.x, sides.y, sides.z });
		if (!std::isfinite(size) || size <= 0.0f) return false;
		m_center = (low + high) * 0.5f;
		m_size = size;
		return true;
	}

	namespace AuxLodGroup
	{
		//the largest scale of a matrix (its axes)
		static float scale(const Mat4& model)
		{
			const float x = length(Vec3(model[0]));
			const float y = length(Vec3(model[1]));
			const float z = length(Vec3(model[2]));
			return std::max({ x, y, z });
		}

		//the share of the height of the screen of an object of a size at a distance: perspective
		//(the view as tall as 2 distance tan(fov / 2), projection[1][1] = 1 / tan(fov / 2)), or
		//orthographic (as tall as 2 / projection[1][1])
		static float screen_share(const Mat4& projection, float size, float distance)
		{
			const bool  perspective = projection[3][3] == 0.0f;
			const float view_height = perspective ? 2.0f * distance / projection[1][1]
			                                      : 2.0f / projection[1][1];
			return size / std::max(view_height, 1e-6f);
		}
	}

	size_t LodGroup::level(const Square::Render::Camera& camera) const
	{
		auto owner = actor().lock();
		if (!owner) return m_levels.size();
		const Mat4& model = owner->global_model_matrix();
		const Vec3  center = Vec3(model * Vec4(m_center, 1.0f));
		const Vec3  eye = Vec3(camera.model()[3]);
		const float distance = length(eye - center);
		switch (m_mode)
		{
		case LodMode::DISTANCE:
		{
			for (size_t i = 0; i != m_levels.size(); ++i)
			{
				if (distance < m_levels[i].m_threshold) return i;
			}
		}
		break;
		case LodMode::SCREEN:
		default:
		{
			const float size = m_size * AuxLodGroup::scale(model);
			const float share = AuxLodGroup::screen_share(camera.projection(), size, distance);
			for (size_t i = 0; i != m_levels.size(); ++i)
			{
				if (share >= m_levels[i].m_threshold) return i;
			}
		}
		break;
		}
		return m_levels.size();
	}

	void LodGroup::refresh()
	{
		m_found = false;
		m_shown = ~size_t(0);
	}

	void LodGroup::select(const Square::Render::Camera& camera)
	{
		if (!found())
		{
			find();
			//(no bounds, or not valid ones)
			if (!(m_size > 0.0f) || !std::isfinite(m_size))
			{
				recalculate_bounds();
			}
		}
		show(level(camera));
	}

	bool LodGroup::found() const
	{
		if (!m_found) return false;
		for (const Renderables& renderables : m_renderables)
		{
			for (const auto& weak_renderable : renderables)
			{
				if (weak_renderable.expired()) return false;
			}
		}
		return true;
	}

	void LodGroup::find()
	{
		m_renderables.clear();
		m_renderables.resize(m_levels.size());
		m_found = true;
		m_shown = ~size_t(0);
		auto owner = actor().lock();
		if (!owner) return;
		for (size_t i = 0; i != m_levels.size(); ++i)
		{
			auto child = owner->child(m_levels[i].m_actor);
			if (!child) continue;
			Renderables& renderables = m_renderables[i];
			child->visit([&renderables](Shared<Actor> part) -> bool
			{
				for (const auto& component : part->components())
				{
					auto renderable = DynamicPointerCast<Render::Renderable, Component>(component.second);
					if (renderable)
					{
						renderables.push_back(renderable);
					}
				}
				return true;
			});
		}
	}

	void LodGroup::show(size_t level)
	{
		if (level == m_shown) return;
		m_shown = level;
		for (size_t i = 0; i != m_renderables.size(); ++i)
		{
			for (const auto& weak_renderable : m_renderables[i])
			{
				if (auto renderable = weak_renderable.lock())
				{
					renderable->lod_shown(i == level);
				}
			}
		}
	}

	//events
	void LodGroup::on_attach(Square::Scene::Actor& entity)
	{
		refresh();
	}

	void LodGroup::on_deattch()
	{
		//(its renderables shown again: no group chooses for them)
		for (const Renderables& renderables : m_renderables)
		{
			for (const auto& weak_renderable : renderables)
			{
				if (auto renderable = weak_renderable.lock())
				{
					renderable->lod_shown(true);
				}
			}
		}
		m_renderables.clear();
		refresh();
	}

	//serialize
	void LodGroup::serialize(Square::Data::Archive& archive)
	{
		Data::serialize(archive, this);
	}
	void LodGroup::serialize_json(Square::Data::JsonValue& archive)
	{
		Data::serialize_json(archive, this);
	}
	//deserialize
	void LodGroup::deserialize(Square::Data::Archive& archive)
	{
		Data::deserialize(archive, this);
	}
	void LodGroup::deserialize_json(Square::Data::JsonValue& archive)
	{
		Data::deserialize_json(archive, this);
	}
}
}
