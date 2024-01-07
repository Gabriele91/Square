//
//  Mesh.cpp
//  Square
//
//  Created by Gabriele Di Bari on 09/04/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#include "Square/Config.h"
#include "Square/Core/Context.h"
#include "Square/Core/Filesystem.h"
#include "Square/Core/ClassObjectRegistration.h"
#include "Square/Render/Mesh.h"
#include "Square/Resource/Mesh.h"
#include "Square/Data/ParserStaticMesh.h"

namespace Square
{
namespace Resource
{
	//////////////////////////////////////////////////////////////
	//Add element to objects
	SQUARE_CLASS_OBJECT_REGISTRATION(Mesh);
	//Registration in context
	void Mesh::object_registration(Context& ctx)
	{
		//factory
		ctx.add_resource<Mesh>({ ".sm3d" });
	}
	//////////////////////////////////////////////////////////////
	//constructor
	Mesh::Mesh(Context& context) : Object(context), ResourceObject(context), Render::Mesh(context) {}
	Mesh::Mesh(Context& context, const std::string& path) : Object(context), ResourceObject(context), Render::Mesh(context) { load(path); }

	// helper type for the visitor #4
	template<class... Ts> struct vertex_visit : Ts... { using Ts::operator()...; };
	// explicit deduction guide 
	template<class... Ts> vertex_visit(Ts...) -> vertex_visit<Ts...>;
	// for type
	template < typename T >
	struct vertex_gpu_build
	{
		Render::Mesh& m_mesh;
		Parser::StaticMesh::Context& m_context;

		vertex_gpu_build
		(
			Render::Mesh& mesh,
			Parser::StaticMesh::Context& context
		)
		: m_mesh(mesh)
		, m_context(context)
		{
		}

		void operator() (const T& value)
		{
			if (m_context.m_index.empty() && m_context.m_submesh.empty())
			{
				m_mesh.build(value);
			}
			else if (m_context.m_submesh.empty())
			{
				m_mesh.build(value, m_context.m_index);
			}
			else if (m_context.m_index.empty())
			{
				m_mesh.build(value, m_context.m_submesh);
			}
			else
			{
				m_mesh.build(value, m_context.m_index, m_context.m_submesh);
			}
		}
	};

	//load effect
	bool Mesh::load(const std::string& path)
	{
		Parser::StaticMesh static_mesh_parser;
		Parser::StaticMesh::Context static_mesh_context;
		//do parsing
		if (!static_mesh_parser.parse(static_mesh_context, Filesystem::binary_file_read_all(path)))
		{
			ResourceObject::context().add_wrong("Static model: " + path);
			return false;
		}
		// Ok?
		bool gpu_build = true;
		//build
		std::visit(vertex_visit {
			[&](auto arg) { gpu_build = false; },
			vertex_gpu_build<Render::Mesh::Vertex2DList>(*this,static_mesh_context),
			vertex_gpu_build<Render::Mesh::Vertex3DList>(*this,static_mesh_context),
			vertex_gpu_build<Render::Mesh::Vertex2DUVList>(*this,static_mesh_context),
			vertex_gpu_build<Render::Mesh::Vertex3DUVList>(*this,static_mesh_context),
			vertex_gpu_build<Render::Mesh::Vertex3DNTBUVList>(*this,static_mesh_context),
        }, static_mesh_context.m_vertex);
		// Return status
		return gpu_build;
	}
}
}
