//
//  SkinnedMesh.cpp
//  Square
//
//  See SkinnedMesh.h for the high level description.
//
#include <algorithm>
#include <limits>
#include "Square/Config.h"
#include "Square/System/RenderSystem.h"
#include "Square/Scene/Actor.h"
#include "Square/Scene/SkinnedMesh.h"
#include "Square/Render/Effect.h"
#include "Square/Render/Material.h"
#include "Square/Resource/Shader.h"
#include "Square/Core/ClassObjectRegistration.h"

namespace Square
{
namespace Scene
{
	SQUARE_CLASS_OBJECT_REGISTRATION(SkinnedMesh);

	//regs
	void SkinnedMesh::object_registration(Square::Context& ctx)
	{
		using namespace Square::Resource;
		ctx.add_object<SkinnedMesh>();
		// Its shadow (false: never in the shadow maps)
		ctx.add_attribute_function<SkinnedMesh, bool>
			("cast_shadow"
			, true
			, [](const SkinnedMesh* renderable) -> bool { return renderable->Render::Renderable::cast_shadow(); }
			, [](SkinnedMesh* renderable, const bool& cast) { renderable->Render::Renderable::cast_shadow(cast); });
		// Mesh
		ctx.add_attribute_function<SkinnedMesh, std::string>
			("mesh"
			, std::string()
			, [](const SkinnedMesh* skinned) -> std::string
			{
				return skinned->m_mesh ? skinned->m_mesh->resource_untyped_name() : std::string();
			}
			, [](SkinnedMesh* skinned, const std::string& name)
			{
				if (name.size())
				{
					skinned->m_mesh = skinned->context().resource<Mesh>(name);
					if (!skinned->m_mesh)
					{
						skinned->context().logger()->warning("SkinnedMesh: unable to load the mesh " + name);
					}
				}
			});
		// Materials
		ctx.add_attribute_function<SkinnedMesh, std::vector<std::string> >
			("materials"
			, std::vector<std::string>()
			, [](const SkinnedMesh* skinned) -> std::vector<std::string>
			{
				std::vector<std::string> names;
				for (const auto& material : skinned->m_materials)
				{
					names.push_back(material ? material->resource_untyped_name() : std::string());
				}
				return names;
			}
			, [](SkinnedMesh* skinned, const std::vector<std::string>& names)
			{
				skinned->m_materials.clear();
				for (const auto& name : names)
				{
					skinned->m_materials.push_back(skinned->context().resource<Material>(name));
				}
			});
		// Joints
		ctx.add_attribute_function<SkinnedMesh, std::vector<std::string> >
			("joints"
			, std::vector<std::string>()
			, [](const SkinnedMesh* skinned) -> std::vector<std::string> { return skinned->m_joint_paths; }
			, [](SkinnedMesh* skinned, const std::vector<std::string>& paths)
			{
				skinned->m_joint_paths = paths;
				skinned->m_joints_found = false;
			});
		ctx.add_attribute_function<SkinnedMesh, std::vector<Mat4> >
			("inverse_binds"
			, std::vector<Mat4>()
			, [](const SkinnedMesh* skinned) -> std::vector<Mat4> { return skinned->m_inverse_binds; }
			, [](SkinnedMesh* skinned, const std::vector<Mat4>& inverse_binds)
			{
				skinned->m_inverse_binds = inverse_binds;
				skinned->m_joints_found = false;
			});
		ctx.add_attribute_function<SkinnedMesh, float>
			("reach"
			, 1.0f
			, [](const SkinnedMesh* skinned) -> float { return skinned->m_reach; }
			, [](SkinnedMesh* skinned, const float& reach) { skinned->reach(reach); });
	}

	SkinnedMesh::SkinnedMesh(Square::Context& context)
	: Component(context)
	, m_joint_matrices(joints_max, Mat4(1.0f))
	{
	}

	SkinnedMesh::~SkinnedMesh()
	{
	}

	void SkinnedMesh::joints(const std::vector<std::string>& paths, const std::vector<Mat4>& inverse_binds)
	{
		m_joint_paths = paths;
		m_inverse_binds = inverse_binds;
		m_joints_found = false;
	}

	void SkinnedMesh::find_joints()
	{
		m_joints.clear();
		auto owner = actor().lock();
		const size_t count = std::min(m_joint_paths.size(), joints_max);
		if (owner && m_joint_paths.size() > joints_max)
		{
			context().logger()->warning("SkinnedMesh: " + owner->name() + ", more than " + std::to_string(joints_max) + " joints");
		}
		for (size_t i = 0; owner && i < count; ++i)
		{
			Shared<Actor> joint = owner->find(m_joint_paths[i]);
			if (!joint)
			{
				context().logger()->warning("SkinnedMesh: " + owner->name() + ", no joint " + m_joint_paths[i]);
			}
			m_joints.push_back(joint);
		}
		m_joints_found = bool(owner);
	}

	void SkinnedMesh::pose()
	{
		if (!m_joints_found)
		{
			find_joints();
		}
		//each joint in the world, by its inverse bind; the box of their places
		Vec3 low(std::numeric_limits<float>::max());
		Vec3 high(-std::numeric_limits<float>::max());
		bool any = false;
		for (size_t i = 0; i < m_joints.size(); ++i)
		{
			if (auto joint = m_joints[i].lock())
			{
				const Mat4& world = joint->global_model_matrix();
				const Mat4 inverse_bind = i < m_inverse_binds.size() ? m_inverse_binds[i] : Mat4(1.0f);
				m_joint_matrices[i] = world * inverse_bind;
				const Vec3 at = Vec3(world[3]);
				low = min(low, at);
				high = max(high, at);
				any = true;
			}
		}
		if (!any)
		{
			//no joint: at its actor
			low = high = Vec3(0.0f);
			if (auto transform = m_transform.lock())
			{
				low = high = Vec3(transform->global_model_matrix()[3]);
			}
		}
		const Vec3 half = (high - low) * 0.5f + Vec3(m_reach);
		m_obb_global.set(Mat3(1.0f), (low + high) * 0.5f, half);
		//a new pose: uploaded again at its next draw
		m_uploaded = false;
	}

	void SkinnedMesh::on_late_update(double delta_time)
	{
		pose();
	}

	size_t SkinnedMesh::materials_count() const
	{
		return m_materials.size();
	}

	Weak<Render::Material> SkinnedMesh::material(size_t i) const
	{
		Weak<Render::Material> material;
		if (i < m_materials.size() && m_materials[i])
		{
			material = DynamicPointerCast<Render::Material>(m_materials[i]);
		}
		return material;
	}

	void SkinnedMesh::draw
	(
		  Render::Context& render
		, size_t material_id
		, Render::EffectPassInputs& input
		, Render::EffectPass& pass
		, int draw_id
	)
	{
		const bool drawable = m_mesh && material_id < m_materials.size() && m_materials[material_id] && material_id < m_mesh->number_of_sub_meshs();
		//the buffer of its joints (made once: the size of Skin.hlsl)
		if (drawable && !m_buffer)
		{
			m_buffer = Render::stream_constant_buffer(&render, sizeof(Mat4) * joints_max);
		}
		if (drawable && m_buffer)
		{
			pass.bind(render, input, m_materials[material_id]->parameters(), draw_id);
			if (auto uniform = pass.m_shader->constant_buffer("Skin"))
			{
				//its joints once a frame (the passes after the first share them), only the ones it has
				if (!m_uploaded)
				{
					const size_t joints = std::max<size_t>(m_joints.size(), 1);
					render.update_steam_CB(m_buffer.get(), (const unsigned char*)m_joint_matrices.data(), sizeof(Mat4) * joints);
					m_uploaded = true;
				}
				uniform->bind(m_buffer.get());
				m_mesh->draw(render, material_id, (unsigned int)pass.m_instances);
			}
			pass.unbind();
		}
	}

	bool SkinnedMesh::visible() const
	{
		return Render::Renderable::visible() && m_mesh;
	}

	bool SkinnedMesh::support_culling() const
	{
		return true;
	}

	const Geometry::OBoundingBox& SkinnedMesh::bounding_box()
	{
		return m_obb_global;
	}

	Weak<Render::Transform> SkinnedMesh::transform() const
	{
		return m_transform;
	}

	//events
	void SkinnedMesh::on_attach(Actor& entity)
	{
		m_transform = DynamicPointerCast<Render::Transform>(entity.shared_from_this());
		m_joints_found = false;
	}

	void SkinnedMesh::on_deattch()
	{
		m_transform.reset();
		m_joints.clear();
		m_joints_found = false;
	}

	//serialize
	void SkinnedMesh::serialize(Data::Archive& archive)
	{
		Data::serialize(archive, this);
	}

	void SkinnedMesh::serialize_json(Data::JsonValue& archive)
	{
		Data::serialize_json(archive, this);
	}

	void SkinnedMesh::deserialize(Data::Archive& archive)
	{
		Data::deserialize(archive, this);
	}

	void SkinnedMesh::deserialize_json(Data::JsonValue& archive)
	{
		Data::deserialize_json(archive, this);
	}
}
}
