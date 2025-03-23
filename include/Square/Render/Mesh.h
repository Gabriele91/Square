//
//  Mesh.h
//  Square
//
//  Created by Gabriele on 02/07/16.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Driver/Render.h"
#include "Square/Render/VertexLayout.h"

namespace Square
{
namespace Render
{
	struct SQUARE_API SubMesh
	{
		Render::DrawType m_draw_type{ Render::DRAW_TRIANGLES };
		unsigned int     m_index_count{ 0 };
		unsigned int     m_index_offset{ 0 };

		SubMesh() = default;

		SubMesh(unsigned int count);
		
		SubMesh(Render::DrawType type, unsigned int count);

		SubMesh(unsigned int count, unsigned int offset);

		SubMesh(Render::DrawType type, unsigned int count, unsigned int offset);
	};

	class SQUARE_API Mesh : public Object,
						    public InheritableSharedObject<Mesh>
	{		

	public:
		//alias
		using Vertex2DList      = std::vector< Layout::Position2D >;
		using Vertex3DList      = std::vector< Layout::Position3D >;
		using Vertex2DUVList    = std::vector< Layout::Position2DUV >;
		using Vertex3DUVList    = std::vector< Layout::Position3DUV >;
		using Vertex3DNUVList   = std::vector< Layout::Position3DNormalUV >;
		using Vertex3DNTBUVList = std::vector< Layout::Position3DNormalTangetBinomialUV >;
		using IndexList   = std::vector<unsigned int>;
		using SubMeshList = std::vector< SubMesh >;

		//implement
		SQUARE_OBJECT(Mesh)

		//init
		Mesh(Square::Context& context);
		virtual ~Mesh();

		//build
		bool build(const Vertex2DList& vertexs, bool cpu_access = false);
		bool build(const Vertex3DList& vertexs, bool cpu_access = false);
		bool build(const Vertex2DUVList& vertexs, bool cpu_access = false);
		bool build(const Vertex3DUVList& vertexs, bool cpu_access = false);
		bool build(const Vertex3DNUVList& vertexs, bool cpu_access = false);
		bool build(const Vertex3DNTBUVList& vertexs, bool cpu_access = false);

		bool build(const Vertex2DList& vertexs, const SubMeshList& submeshs, bool cpu_access = false);
		bool build(const Vertex3DList& vertexs, const SubMeshList& submeshs, bool cpu_access = false);
		bool build(const Vertex2DUVList& vertexs, const SubMeshList& submeshs, bool cpu_access = false);
		bool build(const Vertex3DUVList& vertexs, const SubMeshList& submeshs, bool cpu_access = false);
		bool build(const Vertex3DNUVList& vertexs, const SubMeshList& submeshs, bool cpu_access = false);
		bool build(const Vertex3DNTBUVList& vertexs, const SubMeshList& submeshs, bool cpu_access = false);

		bool build(const Vertex2DList& vertexs, const IndexList& indexs, bool cpu_access = false);
		bool build(const Vertex3DList& vertexs, const IndexList& indexs, bool cpu_access = false);
		bool build(const Vertex2DUVList& vertexs, const IndexList& indexs, bool cpu_access = false);
		bool build(const Vertex3DUVList& vertexs, const IndexList& indexs, bool cpu_access = false);
		bool build(const Vertex3DNUVList& vertexs, const IndexList& indexs, bool cpu_access = false);
		bool build(const Vertex3DNTBUVList& vertexs, const IndexList& indexs, bool cpu_access = false);

		bool build(const Vertex2DList& vertexs, const IndexList& indexs, const SubMeshList& submeshs, bool cpu_access = false);
		bool build(const Vertex3DList& vertexs, const IndexList& indexs, const SubMeshList& submeshs, bool cpu_access = false);
		bool build(const Vertex2DUVList& vertexs, const IndexList& indexs, const SubMeshList& submeshs, bool cpu_access = false);
		bool build(const Vertex3DUVList& vertexs, const IndexList& indexs, const SubMeshList& submeshs, bool cpu_access = false);
		bool build(const Vertex3DNUVList& vertexs, const IndexList& indexs, const SubMeshList& submeshs, bool cpu_access = false);
		bool build(const Vertex3DNTBUVList& vertexs, const IndexList& indexs, const SubMeshList& submeshs, bool cpu_access = false);

		//info
		unsigned int layout_type() const;
		Shared<Render::InputLayout> layout() const;
		Shared<Render::VertexBuffer> vertex_buffer() const;
		Shared<Render::IndexBuffer> index_buffer() const;

		//get surfaces
		const SubMeshList& sub_meshs() const;
		size_t number_of_sub_meshs() const { return sub_meshs().size(); }

		//draw all sub meshs
		void draw(Render::Context& render) const;
		void draw(Render::Context& render, size_t sub_mash_id) const;

	protected:
		//build help
		bool build_vertex_layout(Layout::InputLayoutId type);
		//build help
		bool build_vertex_buffer(unsigned char* data, size_t stride, size_t size, bool cpu_access);
		//build help
		bool build_index_buffer(unsigned int* data, size_t size, bool cpu_access);
		//template help
		template< typename T >
		bool build_vertex_buffer(const std::vector<T>& vertexs, bool cpu_access)
		{
			return build_vertex_buffer((unsigned char*)vertexs.data(), sizeof(T), vertexs.size(), cpu_access);
		}
		bool build_index_buffer(const std::vector<unsigned int>& indexs, bool cpu_access)
		{
			return build_index_buffer((unsigned int*)indexs.data(), indexs.size(), cpu_access);
		}
		//gpu
		unsigned int				 m_layout_type;
		Shared<Render::InputLayout>	 m_layout;
		Shared<Render::VertexBuffer> m_vertex_buffer;
		Shared<Render::IndexBuffer>  m_index_buffer;
		//vector of sub meshs
		std::vector< SubMesh > m_sub_meshs;
	};
}
}