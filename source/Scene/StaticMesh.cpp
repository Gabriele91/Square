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
						sm->context().add_wrong("Faild to load static model: " + name);
					if (!sm->m_obb_local.valid() && !sm->build_local_obounding_box())
						sm->context().add_wrong("Faild to compute the OBB: " + name);
				}
			});
	}

	StaticMesh::StaticMesh(Square::Context& context) 
	: Component(context)
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
		if (auto transform = m_transform.lock())
		{
			using namespace Square;
			m_obb_global = m_obb_local * transform->global_model_matrix();
		}
	}

	const Square::Geometry::OBoundingBox& StaticMesh::bounding_box()
	{
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
		on_transform();
	}
		
	void StaticMesh::on_deattch()
	{
		m_transform.reset();
	}
		
	void StaticMesh::on_message(const Square::Scene::Message& msg) { }

	//serialize
	void StaticMesh::serialize(Square::Data::Archive& archivie)  
	{ 
		Square::Data::serialize(archivie, this);
	}
	void StaticMesh::serialize_json(Square::Data::Json& archivie) { }
	//deserialize
	void StaticMesh::deserialize(Square::Data::Archive& archivie) 
	{ 
		Square::Data::deserialize(archivie, this); 
	}
	void StaticMesh::deserialize_json(Square::Data::Json& archivie) { }

	// build bbox
	bool StaticMesh::build_local_obounding_box()
	{
		if(m_mesh)
		if(auto render = context().render())
		if(auto vbuffer = m_mesh->vertex_buffer())
		if(auto gpuvertex = context().render()->copy_buffer_VBO(vbuffer.get()); gpuvertex.size())
		{
			//offset of position:
			size_t vertex_size = sizeof(Render::Layout::Position3DNormalTangetBinomialUV);
			size_t offest_position = offsetof(Render::Layout::Position3DNormalTangetBinomialUV, m_position);
			size_t n_points = render->get_size_VBO(vbuffer.get()) / vertex_size;
			//cube as obb
			m_obb_local = Geometry::obounding_box_from_sequenzial_triangles
			(
				  gpuvertex.data()
				, offest_position
				, vertex_size
				, n_points
			);
			// Ok
			return m_obb_local.valid();
		}
		return false;
	}
}
}