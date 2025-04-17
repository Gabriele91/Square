#pragma once
#include "Square/Config.h"
#include "Square/Math/Linear.h"
#include "Square/Data/ParserUtils.h"
#include "Square/Driver/Render.h"
#include "Square/Render/Mesh.h"
#include <variant>

namespace Square
{
namespace Parser
{
	namespace StaticMeshAux
	{
		// Helper visitor (namespace-scope)
		template<class... Ts>
		struct vertex_visit : Ts... 
		{
			using Ts::operator()...;
		};
		template<class... Ts>
		vertex_visit(Ts...) -> vertex_visit<Ts...>; // Deduction guide (MUST be at namespace scope)
	}

	class SQUARE_API StaticMesh
	{
	public:
		struct SQUARE_API Context
		{
			// Vertexes
			std::variant <
				Render::Mesh::Vertex2DList,
				Render::Mesh::Vertex3DList,
				Render::Mesh::Vertex2DUVList,
				Render::Mesh::Vertex3DUVList,
				Render::Mesh::Vertex3DNUVList,
				Render::Mesh::Vertex3DNTBUVList
			> m_vertex;
			// Index
			Render::Mesh::IndexList m_index;
			Render::Mesh::SubMeshList m_submesh;
			// Visit mesh
			template < class... Ts >
			void visit(Ts... args)
			{
				std::visit(StaticMeshAux::vertex_visit 
				{
					std::forward<Ts>(args)...
				}, m_vertex);
			}
		};

		bool parse(Context& default_context, const std::vector<unsigned char>& binary);

		bool parse(Context& default_context, const unsigned char*& ptr, size_t size);

		bool serialize(Context& context, std::vector<unsigned char>& binary);
	};
}
}