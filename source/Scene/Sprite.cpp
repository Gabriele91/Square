//
//  Sprite.cpp
//  Square
//
//  See Sprite.h for the high level description.
//
#include <algorithm>
#include "Square/Config.h"
#include "Square/Scene/Actor.h"
#include "Square/Scene/Sprite.h"
#include "Square/Render/Material.h"
#include "Square/Core/ClassObjectRegistration.h"

namespace Square
{
namespace Scene
{
	SQUARE_CLASS_OBJECT_REGISTRATION(Sprite);

	//regs
	void Sprite::object_registration(Square::Context& ctx)
	{
		using namespace Square::Resource;
		ctx.add_object<Sprite>();
		// Material
		ctx.add_attribute_function<Sprite, std::string>
			("material"
			, std::string()
			, [](const Sprite* sprite) -> std::string
			{
				return sprite->m_material ? sprite->m_material->resource_untyped_name() : std::string();
			}
			, [](Sprite* sprite, const std::string& name)
			{
				if (name.size())
				{
					sprite->m_material = sprite->context().resource<Material>(name);
					if (!sprite->m_material)
					{
						sprite->context().logger()->warning("Sprite: unable to load the material " + name);
					}
				}
			});
		// Size, color, rotation, frame
		ctx.add_attribute_function<Sprite, Vec2>
			("size"
			, Vec2(1.0f)
			, [](const Sprite* sprite) -> Vec2 { return sprite->m_size; }
			, [](Sprite* sprite, const Vec2& size) { sprite->size(size); });
		ctx.add_attribute_function<Sprite, Vec4>
			("color"
			, Vec4(1.0f)
			, [](const Sprite* sprite) -> Vec4 { return sprite->m_color; }
			, [](Sprite* sprite, const Vec4& color) { sprite->color(color); });
		ctx.add_attribute_function<Sprite, float>
			("rotation"
			, 0.0f
			, [](const Sprite* sprite) -> float { return sprite->m_rotation; }
			, [](Sprite* sprite, const float& rotation) { sprite->rotation(rotation); });
		ctx.add_attribute_function<Sprite, float>
			("frame"
			, -1.0f
			, [](const Sprite* sprite) -> float { return sprite->m_frame; }
			, [](Sprite* sprite, const float& frame) { sprite->frame(frame); });
	}

	Sprite::Sprite(Square::Context& context)
	: Component(context)
	, m_batch(context)
	{
	}

	Sprite::~Sprite()
	{
	}

	void Sprite::size(const Vec2& size)
	{
		m_size = size;
		m_obb_dirty = true;
	}

	size_t Sprite::materials_count() const
	{
		return m_material ? 1 : 0;
	}

	Weak<Render::Material> Sprite::material(size_t i) const
	{
		Weak<Render::Material> material;
		if (i == 0 && m_material)
		{
			material = DynamicPointerCast<Render::Material>(m_material);
		}
		return material;
	}

	void Sprite::draw
	(
		  Render::Context& render
		, size_t material_id
		, Render::EffectPassInputs& input
		, Render::EffectPass& pass
		, int draw_id
	)
	{
		if (material_id == 0 && m_material)
		{
			//its one, at the origin of its actor
			m_sprite.m_center_rotation = Vec4(0.0f, 0.0f, 0.0f, m_rotation);
			m_sprite.m_size_frame = Vec4(m_size.x, m_size.y, m_frame, 0.0f);
			m_sprite.m_color = m_color;
			m_batch.draw(render, pass, input, m_material->parameters(), draw_id, &m_sprite, 1);
		}
	}

	bool Sprite::support_culling() const
	{
		return true;
	}

	const Geometry::OBoundingBox& Sprite::bounding_box()
	{
		if (m_obb_dirty)
		{
			//a cube around it: it turns toward the camera
			const float half = std::max(m_size.x, m_size.y) * 0.5f;
			Geometry::OBoundingBox local;
			local.set(Mat3(1.0f), Vec3(0.0f), Vec3(half));
			m_obb_global = local;
			if (auto transform = m_transform.lock())
			{
				m_obb_global = local * transform->global_model_matrix();
			}
			m_obb_dirty = false;
		}
		return m_obb_global;
	}

	Weak<Render::Transform> Sprite::transform() const
	{
		return m_transform;
	}

	//events
	void Sprite::on_transform()
	{
		m_obb_dirty = true;
	}

	void Sprite::on_attach(Actor& entity)
	{
		m_transform = DynamicPointerCast<Render::Transform>(entity.shared_from_this());
		m_obb_dirty = true;
	}

	void Sprite::on_deattch()
	{
		m_transform.reset();
		m_obb_dirty = true;
	}

	//serialize
	void Sprite::serialize(Data::Archive& archive)
	{
		Data::serialize(archive, this);
	}

	void Sprite::serialize_json(Data::JsonValue& archive)
	{
		Data::serialize_json(archive, this);
	}

	void Sprite::deserialize(Data::Archive& archive)
	{
		Data::deserialize(archive, this);
	}

	void Sprite::deserialize_json(Data::JsonValue& archive)
	{
		Data::deserialize_json(archive, this);
	}
}
}
