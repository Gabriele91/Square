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
	Mesh::Mesh(Square::Context& context) : Object(context) {}

	//build
	void Mesh::build(const Mesh::Vertex2DList& vertexs)
	{
		m_layout = Layout::Collection::index_by_type
		(
			  Layout::LF_POSITION_2D 
		);
		build_vertex_buffer(vertexs);
		m_sub_meshs.emplace_back(vertexs.size());
	}
	void Mesh::build(const Mesh::Vertex3DList& vertexs) 
	{
		m_layout = Layout::Collection::index_by_type
		(
			  Layout::LF_POSITION_3D
		);
		build_vertex_buffer(vertexs);
		m_sub_meshs.emplace_back(vertexs.size());
	}
	void Mesh::build(const Mesh::Vertex2DUVList& vertexs)
	{
		m_layout = Layout::Collection::index_by_type
		(
			  Layout::LF_POSITION_2D
			| Layout::LF_UVMAP
		);
		build_vertex_buffer(vertexs);
		m_sub_meshs.emplace_back(vertexs.size());
	}
	void Mesh::build(const Mesh::Vertex3DUVList& vertexs)
	{
		m_layout = Layout::Collection::index_by_type
		(
			  Layout::LF_POSITION_3D
			| Layout::LF_UVMAP
		);
		build_vertex_buffer(vertexs);
		m_sub_meshs.emplace_back(vertexs.size());
	}
	void Mesh::build(const Mesh::Vertex3DNTBUVList& vertexs) 
	{
		m_layout = Layout::Collection::index_by_type
		(
			  Layout::LF_POSITION_3D
			| Layout::LF_NORMAL
			| Layout::LF_TANGENT
			| Layout::LF_BINOMIAL
			| Layout::LF_UVMAP
		);
		build_vertex_buffer(vertexs);
		m_sub_meshs.emplace_back(vertexs.size());
	}

	void Mesh::build(const Mesh::Vertex2DList& vertexs, const Mesh::SubMeshList& submeshs)
	{
		build(vertexs);
		m_sub_meshs = submeshs;
	}
	void Mesh::build(const Mesh::Vertex3DList& vertexs, const Mesh::SubMeshList& submeshs)
	{
		build(vertexs);
		m_sub_meshs = submeshs;
	}
	void Mesh::build(const Mesh::Vertex2DUVList& vertexs, const Mesh::SubMeshList& submeshs)
	{
		build(vertexs);
		m_sub_meshs = submeshs;
	}
	void Mesh::build(const Mesh::Vertex3DUVList& vertexs, const Mesh::SubMeshList& submeshs)
	{
		build(vertexs);
		m_sub_meshs = submeshs;
	}
	void Mesh::build(const Mesh::Vertex3DNTBUVList& vertexs, const Mesh::SubMeshList& submeshs)
	{
		build(vertexs);
		m_sub_meshs = submeshs;
	}

	void Mesh::build(const Vertex2DList& vertexs, const IndexList& indexs)
	{
		m_layout = Layout::Collection::index_by_type
		(
			Layout::LF_POSITION_2D
		);
		build_vertex_buffer(vertexs);
		build_index_buffer(indexs);
		m_sub_meshs.emplace_back(indexs.size());
	}
	void Mesh::build(const Vertex3DList& vertexs, const IndexList& indexs)
	{
		m_layout = Layout::Collection::index_by_type
		(
			Layout::LF_POSITION_3D
		);
		build_vertex_buffer(vertexs);
		build_vertex_buffer(vertexs);
		m_sub_meshs.emplace_back(indexs.size());
	}
	void Mesh::build(const Vertex2DUVList& vertexs, const IndexList& indexs)
	{
		m_layout = Layout::Collection::index_by_type
		(
			  Layout::LF_POSITION_2D
			| Layout::LF_UVMAP
		);
		build_vertex_buffer(vertexs);
		m_sub_meshs.emplace_back(indexs.size());
	}
	void Mesh::build(const Vertex3DUVList& vertexs, const IndexList& indexs)
	{
		m_layout = Layout::Collection::index_by_type
		(
			  Layout::LF_POSITION_3D
			| Layout::LF_UVMAP
		);
		build_vertex_buffer(vertexs);
		build_index_buffer(indexs);
		m_sub_meshs.emplace_back(indexs.size());
	}
	void Mesh::build(const Vertex3DNTBUVList& vertexs, const IndexList& indexs)
	{
		m_layout = Layout::Collection::index_by_type
		(
			  Layout::LF_POSITION_3D
			| Layout::LF_NORMAL
			| Layout::LF_TANGENT
			| Layout::LF_BINOMIAL
			| Layout::LF_UVMAP
		);
		build_vertex_buffer(vertexs);
		build_index_buffer(indexs);
		m_sub_meshs.emplace_back(indexs.size());
	}

	void Mesh::build(const Vertex2DList& vertexs, const IndexList& indexs, const SubMeshList& submeshs)
	{
		build(vertexs, indexs);
		m_sub_meshs = submeshs;
	}
	void Mesh::build(const Vertex3DList& vertexs, const IndexList& indexs, const SubMeshList& submeshs)
	{
		build(vertexs, indexs);
		m_sub_meshs = submeshs;
	}
	void Mesh::build(const Vertex2DUVList& vertexs, const IndexList& indexs, const SubMeshList& submeshs)
	{
		build(vertexs, indexs);
		m_sub_meshs = submeshs;
	}
	void Mesh::build(const Vertex3DUVList& vertexs, const IndexList& indexs, const SubMeshList& submeshs)
	{
		build(vertexs, indexs);
		m_sub_meshs = submeshs;
	}
	void Mesh::build(const Vertex3DNTBUVList& vertexs, const IndexList& indexs, const SubMeshList& submeshs)
	{
		build(vertexs, indexs);
		m_sub_meshs = submeshs;
	}

	//data info
	size_t Mesh::layout() const
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
		if (m_index_buffer)
		{
			for (auto& sub_mesh : m_sub_meshs)
				render.draw_elements(sub_mesh.m_draw_type, sub_mesh.m_index_offset, sub_mesh.m_index_count);
		}
		else
		{
			for (auto& sub_mesh : m_sub_meshs)
				render.draw_arrays(sub_mesh.m_draw_type, sub_mesh.m_index_offset, sub_mesh.m_index_count);
		}
	}

	//build help
	bool Mesh::build_vertex_buffer(unsigned char* data, size_t stride, size_t size)
	{
		if(auto render = context().render())
			m_vertex_buffer = Render::vertex_buffer(render, data, stride, size);
		return m_vertex_buffer != false;
	}

	//build help
	bool Mesh::build_index_buffer(unsigned int* data, size_t size)
	{
		if (auto render = context().render())
			m_index_buffer = Render::index_buffer(render, data, size);
		return m_index_buffer != false;
	}
}
}