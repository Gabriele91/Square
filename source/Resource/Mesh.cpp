//
//  Mesh.cpp
//  Square
//
//  Created by Gabriele Di Bari on 09/04/18.
//  Copyright � 2018 Gabriele Di Bari. All rights reserved.
//
#include <type_traits>
#include "Square/Config.h"
#include "Square/Core/Context.h"
#include "Square/Core/Filesystem.h"
#include "Square/Core/ClassObjectRegistration.h"
#include "Square/Render/Mesh.h"
#include "Square/Resource/Mesh.h"
#include "Square/Data/ParserStaticMesh.h"
#include "Square/System/RenderSystem.h"
#include "Square/Driver/Render.h"

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
	namespace AuxMeshTriangles
	{
		static Vec3 to_vec3(const Vec2& v) { return Vec3(v, 0.0f); }
		static Vec3 to_vec3(const Vec3& v) { return v; }

		//the vertex of the i-th index of a sub mesh (no indices: in order)
		static size_t vertex_id(const Parser::StaticMesh::Context& mesh, const Render::SubMesh& submesh, unsigned int i)
		{
			size_t id = size_t(submesh.m_index_offset) + i;
			if (!mesh.m_index.empty())
			{
				id = size_t(mesh.m_index[id]);
			}
			return id;
		}

		//the uv of a vertex (none: 0)
		template < class V, class = void >
		struct has_uv : std::false_type {};
		template < class V >
		struct has_uv<V, std::void_t<decltype(std::declval<V&>().m_uvmap)>> : std::true_type {};

		template < class V >
		static Vec2 uv_of(const V& vertex)
		{
			if constexpr (has_uv<V>::value)
			{
				return Vec2(vertex.m_uvmap);
			}
			else
			{
				return Vec2(0.0f);
			}
		}

		//the triangles of a sub mesh (its positions)
		static void add(Vector<Vec3>& out, const Parser::StaticMesh::Context& mesh, const Render::SubMesh& submesh, const Vector<Vec3>& positions)
		{
			for (unsigned int i = 0; i + 2 < submesh.m_index_count; i += 3)
			{
				const size_t a = vertex_id(mesh, submesh, i);
				const size_t b = vertex_id(mesh, submesh, i + 1);
				const size_t c = vertex_id(mesh, submesh, i + 2);
				const size_t count = positions.size();
				if (a < count && b < count && c < count)
				{
					out.push_back(positions[a]);
					out.push_back(positions[b]);
					out.push_back(positions[c]);
				}
			}
		}
	}

	bool Mesh::local_data(Parser::StaticMesh::Context& out) const
	{
		//its file (a mesh made at run time, of no file: none)
		if (!*resource_name())
		{
			return false;
		}
		const String& path = const_cast<Mesh*>(this)->context().resource_path<Mesh>(resource_untyped_name());
		if (path.empty())
		{
			return false;
		}
		const bool compressed = Filesystem::get_extension(path) == ".sm3dgz";
		Vector<unsigned char> bytes;
		if (compressed)
		{
			bytes = Filesystem::binary_compress_file_read_all(path);
		}
		else
		{
			bytes = Filesystem::binary_file_read_all(path);
		}
		return Parser::StaticMesh().parse(out, bytes);
	}

	bool Mesh::local_triangles(Vector<Vec3>& out, const std::function<bool(size_t submesh)>& filter) const
	{
		//its file
		Parser::StaticMesh::Context mesh;
		if (!local_data(mesh))
		{
			return false;
		}
		//its positions
		Vector<Vec3> positions;
		std::visit([&positions](const auto& vertices)
		{
			positions.reserve(vertices.size());
			for (const auto& vertex : vertices)
			{
				positions.push_back(AuxMeshTriangles::to_vec3(vertex.m_position));
			}
		}, mesh.m_vertex);
		//its sub meshes (the whole mesh when there are none)
		Render::Mesh::SubMeshList submeshes = mesh.m_submesh;
		if (submeshes.empty())
		{
			size_t count = positions.size();
			if (!mesh.m_index.empty())
			{
				count = mesh.m_index.size();
			}
			submeshes.push_back(Render::SubMesh(Render::DRAW_TRIANGLES, (unsigned int)count, 0));
		}
		for (size_t submesh_id = 0; submesh_id < submeshes.size(); ++submesh_id)
		{
			const Render::SubMesh& submesh = submeshes[submesh_id];
			const bool triangles = submesh.m_draw_type == Render::DRAW_TRIANGLES;
			const bool taken = !filter || filter(submesh_id);
			if (triangles && taken)
			{
				AuxMeshTriangles::add(out, mesh, submesh, positions);
			}
		}
		return true;
	}

	bool Mesh::local_triangles(Vector<Vec3>& out, Vector<Vec2>& out_uvs, const std::function<bool(size_t submesh)>& filter) const
	{
		//its file
		Parser::StaticMesh::Context mesh;
		if (!local_data(mesh))
		{
			return false;
		}
		//its positions, its uv
		Vector<Vec3> positions;
		Vector<Vec3> uvs;
		std::visit([&positions, &uvs](const auto& vertices)
		{
			positions.reserve(vertices.size());
			uvs.reserve(vertices.size());
			for (const auto& vertex : vertices)
			{
				positions.push_back(AuxMeshTriangles::to_vec3(vertex.m_position));
				uvs.push_back(Vec3(AuxMeshTriangles::uv_of(vertex), 0.0f));
			}
		}, mesh.m_vertex);
		//its sub meshes (the whole mesh when there are none)
		Render::Mesh::SubMeshList submeshes = mesh.m_submesh;
		if (submeshes.empty())
		{
			size_t count = positions.size();
			if (!mesh.m_index.empty())
			{
				count = mesh.m_index.size();
			}
			submeshes.push_back(Render::SubMesh(Render::DRAW_TRIANGLES, (unsigned int)count, 0));
		}
		Vector<Vec3> uvs_out;
		for (size_t submesh_id = 0; submesh_id < submeshes.size(); ++submesh_id)
		{
			const Render::SubMesh& submesh = submeshes[submesh_id];
			const bool triangles = submesh.m_draw_type == Render::DRAW_TRIANGLES;
			const bool taken = !filter || filter(submesh_id);
			if (triangles && taken)
			{
				AuxMeshTriangles::add(out, mesh, submesh, positions);
				AuxMeshTriangles::add(uvs_out, mesh, submesh, uvs);
			}
		}
		out_uvs.clear();
		out_uvs.reserve(uvs_out.size());
		for (const Vec3& uv : uvs_out)
		{
			out_uvs.push_back(Vec2(uv));
		}
		return true;
	}

	//////////////////////////////////////////////////////////////
	//constructor
	Mesh::Mesh(Context& context) : ResourceObject(context), BaseInheritableSharedObject(context.allocator()), m_mesh(context) {}
	Mesh::Mesh(Context& context, const String& path) : ResourceObject(context), BaseInheritableSharedObject(context.allocator()), m_mesh(context) { load(path); }

	//info
	unsigned int Mesh::layout_type() const
	{
		return m_mesh.layout_type();
	}
	Shared<Render::InputLayout> Mesh::layout() const
	{
		return m_mesh.layout();
	}
	Shared<Render::VertexBuffer> Mesh::vertex_buffer() const
	{
		return m_mesh.vertex_buffer();
	}
	Shared<Render::IndexBuffer> Mesh::index_buffer() const
	{
		return m_mesh.index_buffer();
	}

	//get surfaces
	const Render::Mesh::SubMeshList& Mesh::sub_meshs() const
	{
		return m_mesh.sub_meshs();
	}

	//draw all sub meshs
	void Mesh::draw(Render::Context& render, unsigned int instances) const
	{
		m_mesh.draw(render, instances);
	}

	void Mesh::draw(Render::Context& render, size_t sub_mesh_id, unsigned int instances) const
	{
		m_mesh.draw(render, sub_mesh_id, instances);
	}

	// for type
	template < typename T >
	struct vertex_gpu_build
	{
		Render::Mesh& m_mesh;
		Parser::StaticMesh::Context& m_context;
		bool& m_gpu_build_status;
		bool m_cpu_access;

		vertex_gpu_build
		(
			Render::Mesh& mesh,
			Parser::StaticMesh::Context& context,
			bool& gpu_build_status,
			bool cpu_access
		)
		: m_mesh(mesh)
		, m_context(context)
		, m_gpu_build_status(gpu_build_status)
		, m_cpu_access(cpu_access)
		{
		}

		void operator() (const T& value)
		{
			if (m_context.m_index.empty() && m_context.m_submesh.empty())
			{
				m_gpu_build_status = m_mesh.build(value, m_cpu_access);
			}
			else if (m_context.m_submesh.empty())
			{
				m_gpu_build_status = m_mesh.build(value, m_context.m_index, m_cpu_access);
			}
			else if (m_context.m_index.empty())
			{
				m_gpu_build_status = m_mesh.build(value, m_context.m_submesh, m_cpu_access);
			}
			else
			{
				m_gpu_build_status = m_mesh.build(value, m_context.m_index, m_context.m_submesh, m_cpu_access);
			}
		}
	};

	//load effect
	bool Mesh::load(const String& path)
	{
		Parser::StaticMesh static_mesh_parser;
		Parser::StaticMesh::Context static_mesh_context;
		//get ext
		bool is_compress = Filesystem::get_extension(path) == ".sm3dgz";
		Vector<unsigned char> buffer = is_compress
			                                ? Filesystem::binary_compress_file_read_all(path) 
			                                : Filesystem::binary_file_read_all(path);
		//do parsing
		if (!static_mesh_parser.parse(static_mesh_context, buffer))
		{
			ResourceObject::context().logger()->warning("Static model: " + path);
			return false;
		}
		return build(static_mesh_context, false);
	}

	bool Mesh::build(Parser::StaticMesh::Context& data, bool cpu_access)
	{
		// Ok?
		bool gpu_build_status = true;
		//build
		data.visit(
			[&](auto arg) { gpu_build_status = false; },
			vertex_gpu_build<Render::Mesh::Vertex2DList>(m_mesh, data, gpu_build_status, cpu_access),
			vertex_gpu_build<Render::Mesh::Vertex3DList>(m_mesh, data, gpu_build_status, cpu_access),
			vertex_gpu_build<Render::Mesh::Vertex2DUVList>(m_mesh, data, gpu_build_status, cpu_access),
			vertex_gpu_build<Render::Mesh::Vertex3DUVList>(m_mesh, data, gpu_build_status, cpu_access),
			vertex_gpu_build<Render::Mesh::Vertex3DNUVList>(m_mesh, data, gpu_build_status, cpu_access),
			vertex_gpu_build<Render::Mesh::Vertex3DNTBUVList>(m_mesh, data, gpu_build_status, cpu_access),
			vertex_gpu_build<Render::Mesh::Vertex3DNTBUVSkinList>(m_mesh, data, gpu_build_status, cpu_access)
		);
		// Return status
		return gpu_build_status;
	}

	bool Mesh::build_dynamic(const Mesh& source, const Parser::StaticMesh::Context& data)
	{
		//the file of source: its triangles, its data (the collisions on it as on source)
		resource_name(source.resource_name());
		Parser::StaticMesh::Context copy = data;
		m_dynamic = build(copy, true);
		return m_dynamic;
	}

	bool Mesh::update_vertices(const unsigned char* data, size_t size)
	{
		auto buffer = m_mesh.vertex_buffer();
		if (!m_dynamic || !buffer || !data) return false;
		auto render = System::get<RenderSystem>(context())->render();
		if (!render) return false;
		render->update_steam_VBO(buffer.get(), data, size);
		return true;
	}
}
}
