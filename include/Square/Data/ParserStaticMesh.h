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
				Render::Mesh::Vertex3DNTBUVList
			> m_vertex;
			// Index
			Render::Mesh::IndexList m_index;
			Render::Mesh::SubMeshList m_submesh;
		};

		bool parse(Context& default_context, const std::vector<unsigned char>& binary);

		bool parse(Context& default_context, const unsigned char*& ptr, size_t size);
	};
}
}