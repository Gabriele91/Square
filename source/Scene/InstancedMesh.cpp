#include "Square/Config.h"
#include "Square/System/RenderSystem.h"
#include "Square/Scene/Actor.h"
#include "Square/Scene/InstancedMesh.h"
#include "Square/Render/Effect.h"
#include "Square/Resource/Shader.h"
#include "Square/Geometry/AABoundingBox.h"
#include "Square/Core/ClassObjectRegistration.h"
#include <algorithm>

namespace Square
{
namespace Scene
{
	SQUARE_CLASS_OBJECT_REGISTRATION(InstancedMesh);

	//regs
	void InstancedMesh::object_registration(Square::Context& ctx)
	{
		ctx.add_object<InstancedMesh>();
	}

	InstancedMesh::InstancedMesh(Square::Context& context)
	: Component(context)
	{
	}

	InstancedMesh::~InstancedMesh()
	{
	}

	void InstancedMesh::mesh(const Shared<Resource::Mesh>& mesh, const std::vector< Shared<Resource::Material> >& materials)
	{
		m_mesh = mesh;
		m_materials = materials;
	}

	void InstancedMesh::instances(const std::vector<Mat4>& models, const Geometry::OBoundingBox& mesh_box)
	{
		m_instances = models;
		//the box of all of them (in the space of the actor)
		if (!m_instances.empty())
		{
			Geometry::AABoundingBox box = (mesh_box * m_instances[0]).to_aabb();
			for (size_t i = 1; i < m_instances.size(); ++i) box = box.merge((mesh_box * m_instances[i]).to_aabb());
			const Vec3 center = (box.get_min() + box.get_max()) * 0.5f;
			const Vec3 half = (box.get_max() - box.get_min()) * 0.5f;
			m_obb_local.set(Mat3(1.0f), center, half);
		}
		m_obb_dirty = true;
	}

	size_t InstancedMesh::materials_count() const
	{
		return m_materials.size();
	}

	Weak<Render::Material> InstancedMesh::material(size_t i) const
	{
		if (i < m_materials.size()) return DynamicPointerCast<Render::Material>(m_materials[i]);
		return {};
	}

	void InstancedMesh::draw
	(
		  Render::Context& render
		, size_t material_id
		, Render::EffectPassInputs& input
		, Render::EffectPass& pass
		, int draw_id
	)
	{
		if (!m_mesh || m_instances.empty() || material_id >= m_materials.size() || !m_materials[material_id]) return;
		if (material_id >= m_mesh->number_of_sub_meshs()) return;
		//the buffer of the matrices of a draw (made once: the size of Instances.hlsl)
		if (!m_buffer)
		{
			m_buffer = Render::stream_constant_buffer(&render, sizeof(Mat4) * instances_max);
			m_batch.resize(instances_max, Mat4(1.0f));
			if (!m_buffer) return;
		}
		//in batches: their matrices, a draw each
		for (size_t first = 0; first < m_instances.size(); first += instances_max)
		{
			const size_t count = std::min(instances_max, m_instances.size() - first);
			std::copy(m_instances.begin() + first, m_instances.begin() + first + count, m_batch.begin());
			pass.bind(render, input, m_materials[material_id]->parameters(), draw_id);
			if (auto uniform = pass.m_shader->constant_buffer("Instances"))
			{
				render.update_steam_CB(m_buffer.get(), (const unsigned char*)m_batch.data(), sizeof(Mat4) * instances_max);
				uniform->bind(m_buffer.get());
				m_mesh->draw(render, material_id, (unsigned int)count);
			}
			pass.unbind();
		}
	}

	bool InstancedMesh::support_culling() const
	{
		return true;
	}

	bool InstancedMesh::visible() const
	{
		return Render::Renderable::visible() && m_mesh && !m_instances.empty();
	}

	const Geometry::OBoundingBox& InstancedMesh::bounding_box()
	{
		if (m_obb_dirty)
		{
			if (auto transform = m_transform.lock()) m_obb_global = m_obb_local * transform->global_model_matrix();
			else                                     m_obb_global = m_obb_local;
			m_obb_dirty = false;
		}
		return m_obb_global;
	}

	Weak<Render::Transform> InstancedMesh::transform() const
	{
		return m_transform;
	}

	//events
	void InstancedMesh::on_transform()
	{
		m_obb_dirty = true;
	}

	void InstancedMesh::on_attach(Actor& entity)
	{
		m_transform = DynamicPointerCast<Render::Transform>(entity.shared_from_this());
		m_obb_dirty = true;
	}

	void InstancedMesh::on_deattch()
	{
		m_transform.reset();
		m_obb_dirty = true;
	}

	//serialize (not saved: made at runtime)
	void InstancedMesh::serialize(Data::Archive& archive)          {}
	void InstancedMesh::serialize_json(Data::JsonValue& archive)   {}
	void InstancedMesh::deserialize(Data::Archive& archive)        {}
	void InstancedMesh::deserialize_json(Data::JsonValue& archive) {}
}
}
