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
		using namespace Square::Resource;
		ctx.add_object<InstancedMesh>();
		// Mesh
		ctx.add_attribute_function<InstancedMesh, std::string>
			("mesh"
			, std::string()
			, [](const InstancedMesh* im) -> std::string
			{
				std::string name;
				if (im->m_mesh)
				{
					name = im->m_mesh->resource_untyped_name();
				}
				return name;
			}
			, [](InstancedMesh* im, const std::string& name)
			{
				if (name.size())
				{
					im->m_mesh = im->context().resource<Mesh>(name);
					if (!im->m_mesh)
					{
						im->context().logger()->warning("Faild to load instanced model: " + name);
					}
				}
			});
		// Materials
		ctx.add_attribute_function<InstancedMesh, std::vector<std::string> >
			("materials"
			, std::vector<std::string>()
			, [](const InstancedMesh* im) -> std::vector<std::string>
			{
				std::vector<std::string> names;
				names.reserve(im->m_materials.size());
				for (const auto& material : im->m_materials)
				{
					if (material)
					{
						names.emplace_back(material->resource_untyped_name());
					}
				}
				return names;
			}
			, [](InstancedMesh* im, const std::vector<std::string>& names)
			{
				im->m_materials.clear();
				im->m_materials.reserve(names.size());
				for (const auto& name : names)
				{
					im->m_materials.push_back(im->context().resource<Material>(name));
				}
			});
		// The box of the mesh
		ctx.add_attribute_function<InstancedMesh, std::vector<Vec3> >
			("obb"
			, std::vector<Vec3>()
			, [](const InstancedMesh* im) -> std::vector<Vec3>
			{
				const Geometry::OBoundingBox& box = im->m_mesh_box;
				return
				{
					  box.get_rotation_matrix()[0]
					, box.get_rotation_matrix()[1]
					, box.get_rotation_matrix()[2]
					, box.get_position()
					, box.get_extension()
				};
			}
			, [](InstancedMesh* im, const std::vector<Vec3>& data)
			{
				if (data.size() == 5)
				{
					im->mesh_box(Geometry::OBoundingBox(Mat3{ data[0], data[1], data[2] }, data[3], data[4]));
				}
			});
		// The instances
		ctx.add_attribute_function<InstancedMesh, std::vector<Mat4> >
			("instances"
			, std::vector<Mat4>()
			, [](const InstancedMesh* im) -> std::vector<Mat4> { return im->m_instances; }
			, [](InstancedMesh* im, const std::vector<Mat4>& models) { im->instances(models); });
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

	void InstancedMesh::mesh_box(const Geometry::OBoundingBox& box)
	{
		m_mesh_box = box;
		build_local_box();
	}

	void InstancedMesh::instances(const std::vector<Mat4>& models)
	{
		m_instances = models;
		build_local_box();
	}

	void InstancedMesh::build_local_box()
	{
		//the box of all of them (in the space of the actor), from the one of the mesh
		if (!m_instances.empty())
		{
			Geometry::AABoundingBox box = (m_mesh_box * m_instances[0]).to_aabb();
			for (size_t i = 1; i < m_instances.size(); ++i)
			{
				box = box.merge((m_mesh_box * m_instances[i]).to_aabb());
			}
			const Vec3 center = (box.get_min() + box.get_max()) * 0.5f;
			const Vec3 half = (box.get_max() - box.get_min()) * 0.5f;
			m_obb_local.set(Mat3(1.0f), center, half);
		}
		m_obb_dirty = true;
	}

	bool InstancedMesh::triangles(std::vector<Vec3>& out, const std::function<bool(size_t submesh)>& filter)
	{
		bool read = false;
		auto owner = actor().lock();
		std::vector<Vec3> local;
		if (m_mesh && owner)
		{
			read = m_mesh->local_triangles(local, filter);
		}
		if (read)
		{
			//each instance in world space
			const Mat4 actor_model = owner->global_model_matrix();
			out.reserve(out.size() + local.size() * m_instances.size());
			for (const Mat4& instance : m_instances)
			{
				const Mat4 model = actor_model * instance;
				for (const Vec3& point : local)
				{
					out.push_back(Vec3(model * Vec4(point, 1.0f)));
				}
			}
		}
		return read;
	}

	size_t InstancedMesh::materials_count() const
	{
		return m_materials.size();
	}

	Weak<Render::Material> InstancedMesh::material(size_t i) const
	{
		Weak<Render::Material> material;
		if (i < m_materials.size())
		{
			material = DynamicPointerCast<Render::Material>(m_materials[i]);
		}
		return material;
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
		const bool material = material_id < m_materials.size() && m_materials[material_id];
		const bool drawable = m_mesh && !m_instances.empty() && material && material_id < m_mesh->number_of_sub_meshs();
		//the buffer of the matrices of a draw (made once: the size of Instances.hlsl)
		if (drawable && !m_buffer)
		{
			m_buffer = Render::stream_constant_buffer(&render, sizeof(Mat4) * instances_max);
			m_batch.resize(instances_max, Mat4(1.0f));
		}
		if (drawable && m_buffer)
		{
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
			m_obb_global = m_obb_local;
			if (auto transform = m_transform.lock())
			{
				m_obb_global = m_obb_local * transform->global_model_matrix();
			}
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

	//serialize
	void InstancedMesh::serialize(Data::Archive& archive)
	{
		Data::serialize(archive, this);
	}

	void InstancedMesh::serialize_json(Data::JsonValue& archive)
	{
		Data::serialize_json(archive, this);
	}

	void InstancedMesh::deserialize(Data::Archive& archive)
	{
		Data::deserialize(archive, this);
	}

	void InstancedMesh::deserialize_json(Data::JsonValue& archive)
	{
		Data::deserialize_json(archive, this);
	}
}
}
