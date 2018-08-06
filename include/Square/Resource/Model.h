//
//  Square
//
//  Created by Gabriele on 02/07/16.
//  Copyright ? 2016 Gabriele. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Resource.h"
#include "Square/Driver/Render.h"
#include "Square/Resource/Material.h"

namespace Square
{
namespace Resource
{
	struct SubMesh
	{
		unsigned int m_index_offset;
		unsigned int m_index_Count;
		Render::DrawType m_draw_type;
	};

	class SQUARE_API Mesh : public ResourceObject
                          , public SharedObject<Mesh>
	{		

	public:
		//alias
		using SubMeshList = std::vector< SubMesh >;
		using MaterialList = std::vector< Shared<Material> >;

		//init
		Mesh(Context& context);

		//load		
		bool load(const std::string& path) override;

		//draw
		void draw();

		//get surfaces
		SubMeshList& sub_meshs();

		MaterialList& materials();

	protected:

		//gpu
		Render::VertexBuffer* m_vertex_buffer{ nullptr };
		Render::IndexBuffer*  m_index_buffer{ nullptr };
		//vector of surface
		std::vector< SubMesh > m_sub_meshs;
		std::vector< Shared<Material> > m_materials;
	};
}
}