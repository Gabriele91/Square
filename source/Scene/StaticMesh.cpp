#include "Square/Config.h"
#include "Square/Scene/Actor.h"
#include "Square/Scene/StaticMesh.h"
#include "Square/Geometry/CreateBounding.h"
#include "Square/Core/ClassObjectRegistration.h"

namespace Square
{
namespace Scene
{	
	SQUARE_CLASS_OBJECT_REGISTRATION(StaticMesh);

	//regs
	void StaticMesh::object_registration(Square::Context& ctx)
	{
		//using
		using namespace Square;
		using namespace Square::Resource;
		// Add StaticMesh
		ctx.add_object<StaticMesh>();
		// Material
		ctx.add_attribute_function<StaticMesh, std::string>
			("material"
			, std::string()
			, [](const StaticMesh* sm) -> std::string
			{ 
				if (sm->m_material) 
				    return sm->m_material->resource_untyped_name(); 
				else 
				    return ""; 
			}
			, [](StaticMesh* sm, const std::string& name)
			{ 
				if (name.size())
					sm->m_material = sm->context().resource<Material>(name); 
			});
		// OBB
		ctx.add_attribute_function<StaticMesh, std::vector<Vec3> >
			("obb"
			, std::vector<Vec3>()
			, [](const StaticMesh* sm) -> std::vector<Vec3>
			{ 
				if (sm->m_mesh && sm->m_obb_local.valid()) 
				{
					return
					{     sm->m_obb_local.get_rotation_matrix()[0]
						, sm->m_obb_local.get_rotation_matrix()[1]
						, sm->m_obb_local.get_rotation_matrix()[2]
						, sm->m_obb_local.get_position()
						, sm->m_obb_local.get_extension()
					};
				}; 
				return {};
			}
			, [](StaticMesh* sm, const std::vector<Vec3>& obb_data)
			{ 
				if (obb_data.size() == 5)
			    {
					sm->m_obb_local.set(
					Mat3{
						  obb_data[0]
						, obb_data[1]
						, obb_data[2]
						}
						, obb_data[3]
						, obb_data[4]
					);
				}
			});
		// Mesh
		ctx.add_attribute_function<StaticMesh, std::string>
			("mesh"
			, std::string()
			, [](const StaticMesh* sm) -> std::string
			{ if (sm->m_mesh) return sm->m_mesh->resource_untyped_name(); else return ""; }
			, [](StaticMesh* sm, const std::string& name)
			{ if (name.size())
			    {
				    if(sm->m_mesh = sm->context().resource<Mesh>(name); !sm->m_mesh)
						sm->context().logger()->warning("Faild to load static model: " + name);
					if (!sm->m_obb_local.valid() && !sm->build_local_obounding_box())
						sm->context().logger()->warning("Faild to compute the OBB: " + name);
				}
			});
	}

	StaticMesh::StaticMesh(Square::Context& context) 
	: Component(context)
	, m_obb_dirty(true)
	{
		build_local_obounding_box();
	}

	size_t StaticMesh::materials_count() const 
	{ 
		return 1;
	}

	Weak<Square::Render::Material> StaticMesh::material(size_t i) const
	{
		return Square::DynamicPointerCast<Square::Render::Material>(m_material);
	}

	void StaticMesh::draw
	(
		  Square::Render::Context& render
		, size_t material_id
		, Square::Render::EffectPassInputs& input
		, Square::Render::EffectPass& pass
	)
	{

		using namespace Square;
		using namespace Square::Data;
		using namespace Square::Scene;
		using namespace Square::Resource;
		using namespace Square::Render;
		using namespace Square::Render::Layout;
		//bind
		pass.bind(render, input, m_material->parameters());
		//draw
		m_mesh->draw(render);
		//unbind
		pass.unbind();
	}

	bool StaticMesh::support_culling() const
	{
		return true;
	}

	bool StaticMesh::visible() const
	{ 
		return Square::Render::Renderable::visible() && m_mesh;
	}

	void StaticMesh::on_transform()
	{
		m_obb_dirty = true;
	}

	const Square::Geometry::OBoundingBox& StaticMesh::bounding_box()
	{
		if (m_obb_dirty)
		{
			if (auto transform = m_transform.lock())
			{
				using namespace Square;
				m_obb_global = m_obb_local * transform->global_model_matrix();
			}
			else
			{
				m_obb_global = m_obb_local;
			}
			m_obb_dirty = false;
		}
		return m_obb_global;
	}

	Square::Weak<Square::Render::Transform> StaticMesh::transform() const
	{
		return m_transform;
	}

	//events
	void StaticMesh::on_attach(Square::Scene::Actor& entity)
	{
		m_transform = Square::DynamicPointerCast<Square::Render::Transform>(entity.shared_from_this());
		m_obb_dirty = true;
	}
		
	void StaticMesh::on_deattch()
	{
		m_transform.reset();
		m_obb_dirty = true;
	}
		
	void StaticMesh::on_message(const Square::Scene::Message& msg) { }

	//serialize
	void StaticMesh::serialize(Square::Data::Archive& archive)  
	{ 
		Data::serialize(archive, this);
	}
	void StaticMesh::serialize_json(Square::Data::JsonValue& archive)
	{
		Data::serialize_json(archive, this);
	}
	//deserialize
	void StaticMesh::deserialize(Square::Data::Archive& archive) 
	{ 
		Square::Data::deserialize(archive, this); 
	}
	void StaticMesh::deserialize_json(Square::Data::JsonValue& archive)
	{
		Data::deserialize_json(archive, this);
	}

	// build bbox
	bool StaticMesh::build_local_obounding_box(bool from_triangles)
	{
		if(m_mesh)
		if(auto render = context().render())
		if(auto vbuffer = m_mesh->vertex_buffer())
		if(auto gpuvertex = context().render()->copy_buffer_VBO(vbuffer.get()); gpuvertex.size())
		{
			const unsigned int mesh_type = m_mesh->layout_type();
			// Vertex size
			size_t vertex_size = Render::Layout::Collection::size_by_type(m_mesh->layout_type());
			// If 0 
			if (!vertex_size) return false;
			// Position offset
			auto attributes = Render::Layout::Collection::attributes_by_type(m_mesh->layout_type());
			// Test
			if (!attributes.has_value()) return false;
			// Looking for position
			for (auto& attribute : *attributes)
			{
				if (attribute.m_attribute == Render::AttributeType::ATT_POSITION)
				if (attribute.m_strip == Render::AttributeStripType::AST_FLOAT3)
				{
					size_t offest_position = attribute.m_offset;
					size_t n_points = render->get_size_VBO(vbuffer.get()) / vertex_size;
					//cube as obb
					m_obb_local = Geometry::obounding_box_from_points
					(
						 gpuvertex.data()
						, offest_position
						, vertex_size
						, n_points
					);
					// Set OBB as dirty
					m_obb_dirty = true;
					// Return
					return m_obb_local.valid();
				}
			}
		}
		return false;
	}

	void StaticMesh::set_obounding_box(const Square::Geometry::OBoundingBox& obb)
	{
		m_obb_dirty = true;
		m_obb_local = obb;
	}

}
}