//
//  Mesh.cpp
//  Square
//
//  Created by Gabriele Di Bari on 09/04/18.
//  Copyright � 2018 Gabriele Di Bari. All rights reserved.
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
		ctx.add_resource<Mesh>({ ".sm3d", ".sm3dgz"});
	}
	//////////////////////////////////////////////////////////////
	//constructor
	Mesh::Mesh(Context& context) : Object(context), ResourceObject(context), Render::Mesh(context), SharedObject<Mesh>(context.allocator()) {}
	Mesh::Mesh(Context& context, const std::string& path) : Object(context), ResourceObject(context), Render::Mesh(context), SharedObject<Mesh>(context.allocator()) { load(path); }

	// for type
	template < typename T >
	struct vertex_gpu_build
	{
		Render::Mesh& m_mesh;
		Parser::StaticMesh::Context& m_context;
		bool& m_gpu_build_status;

		vertex_gpu_build
		(
			Render::Mesh& mesh,
			Parser::StaticMesh::Context& context,
			bool& gpu_build_status
		)
		: m_mesh(mesh)
		, m_context(context)
		, m_gpu_build_status(gpu_build_status)
		{
		}

		void operator() (const T& value)
		{
			if (m_context.m_index.empty() && m_context.m_submesh.empty())
			{
				m_gpu_build_status = m_mesh.build(value);
			}
			else if (m_context.m_submesh.empty())
			{
				m_gpu_build_status = m_mesh.build(value, m_context.m_index);
			}
			else if (m_context.m_index.empty())
			{
				m_gpu_build_status = m_mesh.build(value, m_context.m_submesh);
			}
			else
			{
				m_gpu_build_status = m_mesh.build(value, m_context.m_index, m_context.m_submesh);
			}
		}
	};

	//load effect
	bool Mesh::load(const std::string& path)
	{
		Parser::StaticMesh static_mesh_parser;
		Parser::StaticMesh::Context static_mesh_context;
		//get ext
		bool is_compress = Filesystem::get_extension(path) == ".sm3dgz";
		std::vector<unsigned char> buffer = is_compress
			                                ? Filesystem::binary_compress_file_read_all(path) 
			                                : Filesystem::binary_file_read_all(path);
		//do parsing
		if (!static_mesh_parser.parse(static_mesh_context, buffer))
		{
			ResourceObject::context().add_wrong("Static model: " + path);
			return false;
		}
		// Ok?
		bool gpu_build_status = true;
		//build
		static_mesh_context.visit(
			[&](auto arg) { gpu_build_status = false; },
			vertex_gpu_build<Render::Mesh::Vertex2DList>(*this, static_mesh_context, gpu_build_status),
			vertex_gpu_build<Render::Mesh::Vertex3DList>(*this, static_mesh_context, gpu_build_status),
			vertex_gpu_build<Render::Mesh::Vertex2DUVList>(*this, static_mesh_context, gpu_build_status),
			vertex_gpu_build<Render::Mesh::Vertex3DUVList>(*this, static_mesh_context, gpu_build_status),
			vertex_gpu_build<Render::Mesh::Vertex3DNTBUVList>(*this, static_mesh_context, gpu_build_status)
        );
		// Return status
		return gpu_build_status;
	}
}
}
