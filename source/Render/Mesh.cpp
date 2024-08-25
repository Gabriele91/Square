#include "Square/Render/Mesh.h"
#include "Square/Driver/Render.h"
#include "Square/Core/Context.h"

namespace Square
{
namespace Render
{
	SubMesh::SubMesh(unsigned int count)
	: m_index_count(count)
	{}
		
	SubMesh::SubMesh(Render::DrawType type, unsigned int count)
	: m_draw_type(type)
	, m_index_count(count)
	{}

	SubMesh::SubMesh(unsigned int count, unsigned int offset)
	: m_index_count(count)
	, m_index_offset(offset) 
	{}

	SubMesh::SubMesh(Render::DrawType type, unsigned int count, unsigned int offset)
	: m_draw_type(type)
	, m_index_count(count)
	, m_index_offset(offset) 
	{}

	//init
	Mesh::Mesh(Square::Context& context) : Object(context), BaseInheritableSharedObject(context.allocator()) {}

	//build
	bool Mesh::build(const Mesh::Vertex2DList& vertexs, bool cpu_access)
	{
		if (!build_vertex_layout(Layout::LF_POSITION_2D)) return false;
		if (!build_vertex_buffer(vertexs, cpu_access)) return false;
		m_sub_meshs.emplace_back(vertexs.size());
		return true;
	}
	bool Mesh::build(const Mesh::Vertex3DList& vertexs, bool cpu_access)
	{
		if (!build_vertex_layout( Layout::LF_POSITION_3D )) return false;
		if (!build_vertex_buffer(vertexs, cpu_access)) return false;
		m_sub_meshs.emplace_back(vertexs.size());
		return true;
	}
	bool Mesh::build(const Mesh::Vertex2DUVList& vertexs, bool cpu_access)
	{
		if (!build_vertex_layout(Layout::LF_POSITION_2D | Layout::LF_UVMAP)) return false;
		if (!build_vertex_buffer(vertexs, cpu_access)) return false;
		m_sub_meshs.emplace_back(vertexs.size());
		return true;
	}
	bool Mesh::build(const Mesh::Vertex3DUVList& vertexs, bool cpu_access)
	{
		if (!build_vertex_layout(Layout::LF_POSITION_3D | Layout::LF_UVMAP)) return false;
		if (!build_vertex_buffer(vertexs, cpu_access)) return false;
		m_sub_meshs.emplace_back(vertexs.size());
		return true;
	}
	bool Mesh::build(const Mesh::Vertex3DNUVList& vertexs, bool cpu_access)
	{
		if (!build_vertex_layout
		(
			Layout::LF_POSITION_3D
			| Layout::LF_NORMAL
			| Layout::LF_UVMAP
		)) return false;
		if (!build_vertex_buffer(vertexs, cpu_access)) return false;
		m_sub_meshs.emplace_back(vertexs.size());
		return true;
	}
	bool Mesh::build(const Mesh::Vertex3DNTBUVList& vertexs, bool cpu_access)
	{
		if (!build_vertex_layout
		(
			  Layout::LF_POSITION_3D
			| Layout::LF_NORMAL
			| Layout::LF_TANGENT
			| Layout::LF_BINOMIAL
			| Layout::LF_UVMAP
		)) return false;
		if(!build_vertex_buffer(vertexs, cpu_access)) return false;
		m_sub_meshs.emplace_back(vertexs.size());
		return true;
	}

	bool Mesh::build(const Mesh::Vertex2DList& vertexs, const Mesh::SubMeshList& submeshs, bool cpu_access)
	{
		if (!build(vertexs, cpu_access)) return false;
		m_sub_meshs = submeshs;
		return true;
	}
	bool Mesh::build(const Mesh::Vertex3DList& vertexs, const Mesh::SubMeshList& submeshs, bool cpu_access)
	{
		if (!build(vertexs, cpu_access)) return false;
		m_sub_meshs = submeshs;
		return true;
	}
	bool Mesh::build(const Mesh::Vertex2DUVList& vertexs, const Mesh::SubMeshList& submeshs, bool cpu_access)
	{
		if (!build(vertexs, cpu_access)) return false;
		m_sub_meshs = submeshs;
		return true;
	}
	bool Mesh::build(const Mesh::Vertex3DUVList& vertexs, const Mesh::SubMeshList& submeshs, bool cpu_access)
	{
		if (!build(vertexs, cpu_access)) return false;
		m_sub_meshs = submeshs;
		return true;
	}
	bool Mesh::build(const Mesh::Vertex3DNUVList& vertexs, const Mesh::SubMeshList& submeshs, bool cpu_access)
	{
		if (!build(vertexs, cpu_access)) return false;
		m_sub_meshs = submeshs;
		return true;
	}
	bool Mesh::build(const Mesh::Vertex3DNTBUVList& vertexs, const Mesh::SubMeshList& submeshs, bool cpu_access)
	{
		if (!build(vertexs, cpu_access)) return false;
		m_sub_meshs = submeshs;
		return true;
	}

	bool Mesh::build(const Vertex2DList& vertexs, const IndexList& indexs, bool cpu_access)
	{
		if (!build_vertex_layout
		(
			Layout::LF_POSITION_2D
		)) return false;
		if (!build_vertex_buffer(vertexs, cpu_access)) return false;
		if (!build_index_buffer(indexs, cpu_access)) return false;
		m_sub_meshs.emplace_back(indexs.size());
		return true;
	}
	bool Mesh::build(const Vertex3DList& vertexs, const IndexList& indexs, bool cpu_access)
	{
		if (!build_vertex_layout
		(
			Layout::LF_POSITION_3D
		)) return false;
		if (!build_vertex_buffer(vertexs, cpu_access)) return false;
		if (!build_index_buffer(indexs, cpu_access)) return false;
		m_sub_meshs.emplace_back(indexs.size());
		return true;
	}
	bool Mesh::build(const Vertex2DUVList& vertexs, const IndexList& indexs, bool cpu_access)
	{
		if (!build_vertex_layout
		(
			  Layout::LF_POSITION_2D
			| Layout::LF_UVMAP
		)) return false;
		if (!build_vertex_buffer(vertexs, cpu_access)) return false;
		if (!build_index_buffer(indexs, cpu_access)) return false;
		m_sub_meshs.emplace_back(indexs.size());
		return true;
	}
	bool Mesh::build(const Vertex3DUVList& vertexs, const IndexList& indexs, bool cpu_access)
	{
		if (!build_vertex_layout
		(
			  Layout::LF_POSITION_3D
			| Layout::LF_UVMAP
		)) return false;
		if (!build_vertex_buffer(vertexs, cpu_access)) return false;
		if (!build_index_buffer(indexs, cpu_access)) return false;
		m_sub_meshs.emplace_back(indexs.size());
		return true;
	}
	bool Mesh::build(const Vertex3DNUVList& vertexs, const IndexList& indexs, bool cpu_access)
	{
		if (!build_vertex_layout
		(
			Layout::LF_POSITION_3D
			| Layout::LF_NORMAL
			| Layout::LF_UVMAP
		)) return false;
		if (!build_vertex_buffer(vertexs, cpu_access)) return false;
		if (!build_index_buffer(indexs, cpu_access)) return false;
		m_sub_meshs.emplace_back(indexs.size());
		return true;
	}
	bool Mesh::build(const Vertex3DNTBUVList& vertexs, const IndexList& indexs, bool cpu_access)
	{
		if (!build_vertex_layout
		(
			  Layout::LF_POSITION_3D
			| Layout::LF_NORMAL
			| Layout::LF_TANGENT
			| Layout::LF_BINOMIAL
			| Layout::LF_UVMAP
		)) return false;
		if (!build_vertex_buffer(vertexs, cpu_access)) return false;
		if (!build_index_buffer(indexs, cpu_access)) return false;
		m_sub_meshs.emplace_back(indexs.size());
		return true;
	}

	bool Mesh::build(const Vertex2DList& vertexs, const IndexList& indexs, const SubMeshList& submeshs, bool cpu_access)
	{
		if (!build(vertexs, indexs, cpu_access)) return false;
		m_sub_meshs = submeshs;
		return true;
	}
	bool Mesh::build(const Vertex3DList& vertexs, const IndexList& indexs, const SubMeshList& submeshs, bool cpu_access)
	{
		if (!build(vertexs, indexs, cpu_access)) return false;
		m_sub_meshs = submeshs;
		return true;
	}
	bool Mesh::build(const Vertex2DUVList& vertexs, const IndexList& indexs, const SubMeshList& submeshs, bool cpu_access)
	{
		if (!build(vertexs, indexs, cpu_access)) return false;
		m_sub_meshs = submeshs;
		return true;
	}
	bool Mesh::build(const Vertex3DUVList& vertexs, const IndexList& indexs, const SubMeshList& submeshs, bool cpu_access)
	{
		if (!build(vertexs, indexs, cpu_access)) return false;
		m_sub_meshs = submeshs;
		return true;
	}
	bool Mesh::build(const Vertex3DNUVList& vertexs, const IndexList& indexs, const SubMeshList& submeshs, bool cpu_access)
	{
		if (!build(vertexs, indexs, cpu_access)) return false;
		m_sub_meshs = submeshs;
		return true;
	}
	bool Mesh::build(const Vertex3DNTBUVList& vertexs, const IndexList& indexs, const SubMeshList& submeshs, bool cpu_access)
	{
		if (!build(vertexs, indexs, cpu_access)) return false;
		m_sub_meshs = submeshs;
		return true;
	}

	//data info
	Shared<Render::InputLayout> Mesh::layout() const
	{
		return m_layout;
	}
	Shared<Render::VertexBuffer> Mesh::vertex_buffer() const
	{
		return m_vertex_buffer;
	}
	Shared<Render::IndexBuffer> Mesh::index_buffer() const
	{
		return m_index_buffer;
	}

	//get surfaces
	const Mesh::SubMeshList& Mesh::sub_meshs() const
	{
		return m_sub_meshs;
	}


	//draw sub mesh
	void Mesh::draw(Render::Context& render) const
	{
		//test
		if (!layout().get() || !vertex_buffer().get())
			return;
		//bind input layout
		render.bind_IL(layout().get());
		//bind vertex buffer
		render.bind_VBO(vertex_buffer().get());
		//draw elements or array
		if (index_buffer().get())
		{
			// Bind index buffer
			render.bind_IBO(index_buffer().get());
			// Draw elements
			for (auto& sub_mesh : m_sub_meshs)
				render.draw_elements(sub_mesh.m_draw_type, sub_mesh.m_index_offset, sub_mesh.m_index_count);
		}
		else
		{
			// Draw array
			for (auto& sub_mesh : m_sub_meshs)
				render.draw_arrays(sub_mesh.m_draw_type, sub_mesh.m_index_offset, sub_mesh.m_index_count);
		}
	}

	//build help
	bool Mesh::build_vertex_layout(Layout::InputLayoutId type)
	{
		if (auto render = context().render())
			m_layout = Layout::Collection::index_by_type(render, type);
		return m_layout != nullptr;
	}

	//build help
	bool Mesh::build_vertex_buffer(unsigned char* data, size_t stride, size_t size, bool cpu_access)
	{
		if (auto render = context().render())
		{
			if (cpu_access)
			{
				m_vertex_buffer = Render::stream_vertex_buffer(render, stride, size);
				if(m_vertex_buffer) render->update_steam_VBO(m_vertex_buffer.get(), data, size);
			}
			else
			{
				m_vertex_buffer = Render::vertex_buffer(render, data, stride, size);
			}
		}
		return m_vertex_buffer != nullptr;
	}

	//build help
	bool Mesh::build_index_buffer(unsigned int* data, size_t size, bool cpu_access)
	{
		if (auto render = context().render())
		{
			if (cpu_access)
			{
				m_index_buffer = Render::stream_index_buffer(render, size);
				if (m_index_buffer) render->update_steam_IBO(m_index_buffer.get(), data, size);
			}
			else
			{
				m_index_buffer = Render::index_buffer(render, data, size);
			}
		}
		return m_index_buffer != nullptr;
	}
}
}
